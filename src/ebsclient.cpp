﻿#include <stdio.h>

#include "AbisRest.h"
#include "ebsclient.h"
#include "imgutils.h"
#include "fplibclient.h"

interprocess_semaphore mx_logger(1);
interprocess_semaphore mx_finder(1);
vector<interprocess_semaphore*> mx_ports;
vector<tcp::socket*> extract_sockets;

boost::asio::io_context io_context_;
static int current_port_ = 0;

int find_free_port()
{
	if (mx_ports.size() == 0)
	{
		for (int i = 0; i < extract_port_count; i++)
		{
			interprocess_semaphore* sem = new interprocess_semaphore(1);
			mx_ports.push_back(sem);

			tcp::socket* client_socket = new tcp::socket(io_context_);
			extract_sockets.push_back(client_socket);
		}
	}

	mx_finder.wait();

	bool one = true;
	while (true)
	{
		int port = current_port_++ % extract_port_count;
		if (mx_ports[port]->try_wait())
		{
			mx_finder.post();
			return port;
		}
		this_thread::yield();
		if (one) BOOST_LOG_TRIVIAL(debug) << "extract face service busy on all ports";

		this_thread::sleep_for(milliseconds(5 * rand() % 5));
		one = false;
	};

	mx_finder.post();
	return -1;
}

int ebs_request(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, size_t template_buf_size,
	const unsigned char cmd, const unsigned char check, const size_t offset)
{
	int port_index = find_free_port();
	if (port_index < 0) return -1;

	int res = 0;
	int current_port = port_index + extract_port_start;

	auto start = steady_clock::now();

	tcp::socket* client_socket = extract_sockets.at(port_index);
	try
	{
		boost::system::error_code err = boost::asio::error::would_block;

		tcp::endpoint ep = tcp::endpoint(ip::address::from_string(extract_host), current_port);
		client_socket->connect(ep, err);

		bool step = !err.failed() && client_socket->is_open();
		if (!step)
		{
			BOOST_LOG_TRIVIAL(debug) << "ebs_request: extract service not response. " << err.message();
			res = -1;
		}

		size_t send_data_len = image_data_len + 1;
		if (step)
		{
			unsigned char send_header[8];
			send_header[0] = 'r';
			send_header[1] = 'e';
			send_header[2] = 'q';
			send_header[3] = 'f';
			send_header[4] = (unsigned char)((send_data_len >> 24) & 0xFF);
			send_header[5] = (unsigned char)((send_data_len >> 16) & 0xFF);
			send_header[6] = (unsigned char)((send_data_len >> 8) & 0xFF);
			send_header[7] = (unsigned char)(send_data_len & 0xFF);

			client_socket->write_some(buffer(send_header, 8), err);
			step = !err.failed();
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: send header error. " << err.message();
				res = -2;
			}
		}

		if (step)
		{
			client_socket->write_some(buffer(image_data, image_data_len), err);
			step = !err.failed();
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: send image error. " << err.message();
				res = -3;
			}
		}

		if (step)
		{
			client_socket->write_some(buffer(&cmd, 1), err);
			step = !err.failed();
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: send cmd error. " << err.message();
				res = -4;
			}
		}

		unsigned char recv_header[8];
		if (step)
		{
			size_t io_len = client_socket->read_some(buffer(recv_header, 8), err);
			step = !err.failed() && io_len == 8;
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: receive header error. " << err.message();
				res = -5;
			}
		}

		if (step)
		{
			step = recv_header[0] == 'a' && recv_header[1] == 'n' && recv_header[2] == 's' && recv_header[3] == 'w';
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: receive header format error. " << err.message();
				res = -6;
			}
		}

		size_t recv_data_len = 0;
		if (step)
		{
			unsigned char p1 = recv_header[4];
			unsigned char p2 = recv_header[5];
			unsigned char p3 = recv_header[6];
			unsigned char p4 = recv_header[7];
			recv_data_len = (size_t)(p1) * 16777216 + (size_t)(p2) * 65536 + (size_t)(p3) * 256 + (size_t)(p4);

			step = recv_data_len > 0;
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: receive header datalen error. " << err.message();
				res = -7;
			}
		}

		unsigned char* recv_data = nullptr;
		if (step)
		{
			recv_data = (unsigned char*)malloc(recv_data_len);
			step = recv_data != nullptr;
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: receive data allocate memory error. " << err.message();
				res = -8;
			}
		}

		if (step)
		{
			memset(recv_data, 0, recv_data_len);
			size_t io_len = client_socket->read_some(buffer(recv_data, recv_data_len), err);

			step = !err.failed() && io_len == recv_data_len && offset + template_buf_size <= recv_data_len;
			if (!step)
			{
				BOOST_LOG_TRIVIAL(debug) << "ebs_request: receive data size error. " << err.message();
				res = -9;
			}
		}
		if (step && cmd == EBS_CMD_EXTRACT_FINGER)
		{
			auto p_ptr = recv_data + 1 + ABIS_TEMPLATE_SIZE;
			step = *(int*)p_ptr > 12;
			if (!step) res = -10;
		}

		if (step)
		{
			memcpy(template_buf, &recv_data[offset], template_buf_size);
			step = recv_data[0] == check;

			if (step) res = 1;
			if (!step && recv_data[0] > 1) res = -11;
			if (!step && recv_data[0] < 1) res = -12;
		}
		if (recv_data != nullptr) free(recv_data);
	}
	catch (const boost::system::error_code& ec)
	{
		BOOST_LOG_TRIVIAL(error) << "ebs_request: " << ec.message();
		res = -100;
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(error) << "ebs_request: " << ec.what();
		res = -101;
	}

	client_socket->close();
	mx_ports[port_index]->post();

	auto diff = steady_clock::now() - start;
	BOOST_LOG_TRIVIAL(debug) << "extract: "
		<< " image size: " << image_data_len
		<< " duration: " << duration_cast<seconds>(diff).count() << "s " << duration_cast<milliseconds>(diff % seconds(1)).count() << "ms"
		<< " res: " << res;

	return res;

}

int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size)
{
	return ebs_request(image_data, image_data_len, template_buf, template_buf_size,
		EBS_CMD_EXTRACT_FACE, EBS_CHECK_EXTRACT_FACE, 1);
}

int extract_finger_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, bool gost)
{
	int res = -1;

	size_t c_image_data_len = 0;
	unsigned char* c_image_data = nullptr;
	convert_image(image_data, image_data_len, c_image_data, c_image_data_len);

	res = ebs_request(c_image_data, c_image_data_len, template_buf, template_buf_size,
		EBS_CMD_EXTRACT_FINGER, EBS_CHECK_EXTRACT_FINGER, (gost ? ABIS_TEMPLATE_SIZE + 1 : 1));

	if (c_image_data != nullptr) free(c_image_data);
	return res;
}

int extract_finger_xyt(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, void* gost_template_buf, const size_t gost_template_buf_size)
{
	int res = 0;

	size_t c_image_data_len = 0;
	unsigned char* c_image_data = nullptr;
	convert_image(image_data, image_data_len, c_image_data, c_image_data_len);

	void* pptr = nullptr;
	pptr = malloc(template_buf_size + gost_template_buf_size + 1);
	if (pptr != nullptr)
	{
		res = ebs_request(c_image_data, c_image_data_len, pptr, template_buf_size + gost_template_buf_size + 1,
			EBS_CMD_EXTRACT_FINGER_XYT, EBS_CHECK_EXTRACT_FINGER, 0);
		if (res > 0) memcpy(gost_template_buf, (uchar*)pptr + 1 + template_buf_size, gost_template_buf_size);
	}
	if (pptr != nullptr) free(pptr);

	if (c_image_data != nullptr) free(c_image_data);
	return res;
}

int extract_finger_templates(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, void* gost_template_buf, const size_t gost_template_buf_size)
{
	int res = 0;

	size_t c_image_data_len = 0;
	unsigned char* c_image_data = nullptr;
	convert_image(image_data, image_data_len, c_image_data, c_image_data_len);

	void* pptr = nullptr;
	pptr = malloc(template_buf_size + gost_template_buf_size + 1);
	if (pptr != nullptr)
	{
		res = ebs_request(c_image_data, c_image_data_len, pptr, template_buf_size + gost_template_buf_size + 1,
			EBS_CMD_EXTRACT_FINGER, EBS_CHECK_EXTRACT_FINGER, 0);
		if (res > 0)
		{
			memcpy(template_buf, (uchar*)pptr + 1, template_buf_size);
			memcpy(gost_template_buf, (uchar*)pptr + 1 + template_buf_size, gost_template_buf_size);
		}
	}
	if (pptr != nullptr) free(pptr);

	if (c_image_data != nullptr) free(c_image_data);
	return res;
}

int extract_iris_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size)
{
	return ebs_request(image_data, image_data_len, template_buf, template_buf_size,
		EBS_CMD_EXTRACT_IRIS, EBS_CHECK_EXTRACT_IRIS, 1);
}

float fvec_eq_dis(const float* x, const float* y, size_t size)
{
	float res = 0;
	for (size_t i = 0; i < size; i++) {
		const float tmp = x[i] - y[i];
		res += tmp * tmp;
	}
	return sqrt(res);
}

float cmp_face_tmp(const void* tmp1, const void* tmp2) {
	float score = 1.0f - min(fvec_eq_dis((const float*)tmp1, (const float*)tmp2, ABIS_TEMPLATE_LEN), 2.0f) / 2.0f;
	BOOST_LOG_TRIVIAL(debug) << __func__ << ": score = " << score;
	return score;
}

float cmp_finger_tmp(const void* tmp1, const void* tmp2) {
	float score = 1.0f - min(fvec_eq_dis((const float*)tmp1, (const float*)tmp2, ABIS_TEMPLATE_LEN), 2.0f) / 2.0f;
	BOOST_LOG_TRIVIAL(debug) << __func__ << ": score = " << score;
	return score;
}

float cmp_iris_tmp(const void* tmp1, const void* tmp2) {
	float score = 1.0f - min(fvec_eq_dis((const float*)tmp1, (const float*)tmp2, ABIS_TEMPLATE_LEN), 2.0f) / 2.0f;
	BOOST_LOG_TRIVIAL(debug) << __func__ << ": score = " << score;
	return score;
}

float sugeno_weber(const float x, const float y)
{
	float score = max((x + y + x * y * SW_NORM_P - 1) / (SW_NORM_P + 1), 0.0f);
	BOOST_LOG_TRIVIAL(debug) << __func__ << ": score = " << score;
	return score;
}

float multi_score(const float x, const float y)
{
	float z = 0.f;
	float f00_1 = 0.f;
	float f01_1 = ABIS_INTEGRA_THRESHOLD / 4.f;
	float f10_1 = ABIS_INTEGRA_THRESHOLD / 4.f;
	float f11_1 = ABIS_INTEGRA_THRESHOLD * 0.8f;
	float b1_1 = f00_1;
	float b2_1 = f10_1 - f00_1;
	float b3_1 = f01_1 - f00_1;
	float b4_1 = f00_1 - f10_1 - f01_1 + f11_1;
	float f00_2 = ABIS_INTEGRA_THRESHOLD / 4.f;
	float f01_2 = ABIS_INTEGRA_THRESHOLD * 0.8f;
	float f10_2 = ABIS_INTEGRA_THRESHOLD / 2.f;
	float f11_2 = ABIS_INTEGRA_THRESHOLD - ABIS_FLOAT_THRESHOLD;
	float b1_2 = f00_2;
	float b2_2 = f10_2 - f00_2;
	float b3_2 = f01_2 - f00_2;
	float b4_2 = f00_2 - f10_2 - f01_2 + f11_2;
	float f00_4 = ABIS_INTEGRA_THRESHOLD;
	float f01_4 = ABIS_INTEGRA_THRESHOLD * 3.f / 2.f;
	float f10_4 = ABIS_INTEGRA_THRESHOLD * 3.f / 2.f;
	float f11_4 = ABIS_INTEGRA_THRESHOLD * 2.f;
	float b1_4 = f00_4;
	float b2_4 = f10_4 - f00_4;
	float b3_4 = f01_4 - f00_4;
	float b4_4 = f00_4 - f10_4 - f01_4 + f11_4;

	if (x < ABIS_INTEGRA_THRESHOLD && abs(ABIS_INTEGRA_THRESHOLD - x) >= ABIS_FLOAT_THRESHOLD
		&& y < ABIS_INTEGRA_THRESHOLD
		&& abs(ABIS_INTEGRA_THRESHOLD - y) >= ABIS_FLOAT_THRESHOLD)
	{
		z = b1_1 + b2_1 * 2.f * x + b3_1 * 2.f * y + b4_1 * 4.f * x * y;
	}
	else if ((x >= ABIS_INTEGRA_THRESHOLD || abs(ABIS_INTEGRA_THRESHOLD - x) < ABIS_FLOAT_THRESHOLD)
		&& y < ABIS_INTEGRA_THRESHOLD
		&& abs(ABIS_INTEGRA_THRESHOLD - y) >= ABIS_FLOAT_THRESHOLD)
	{
		z = b1_2 + b2_2 * 2.f * (x - ABIS_INTEGRA_THRESHOLD) + b3_2 * 2.f * y + b4_2 * 4.f * (x - ABIS_INTEGRA_THRESHOLD) * y;
	}
	else if (x < ABIS_INTEGRA_THRESHOLD && abs(ABIS_INTEGRA_THRESHOLD - x) >= ABIS_FLOAT_THRESHOLD
		&& (y >= ABIS_INTEGRA_THRESHOLD || abs(ABIS_INTEGRA_THRESHOLD - y) < ABIS_FLOAT_THRESHOLD))
	{
		z = b1_2 + b2_2 * 2.f * (y - ABIS_INTEGRA_THRESHOLD) + b3_2 * 2.f * x + b4_2 * 4.f * x * (y - ABIS_INTEGRA_THRESHOLD);
	}
	else if ((x >= ABIS_INTEGRA_THRESHOLD || abs(ABIS_INTEGRA_THRESHOLD - x) < ABIS_FLOAT_THRESHOLD)
		&& (y >= ABIS_INTEGRA_THRESHOLD || abs(ABIS_INTEGRA_THRESHOLD - y) < ABIS_FLOAT_THRESHOLD))
	{
		z = b1_4 + b2_4 * 2.f * (x - ABIS_INTEGRA_THRESHOLD) + b3_4 * 2.f * (y - ABIS_INTEGRA_THRESHOLD)
			+ b4_4 * 4.f * (x - ABIS_INTEGRA_THRESHOLD) * (y - ABIS_INTEGRA_THRESHOLD);
	}

	BOOST_LOG_TRIVIAL(debug) << "multi_score: x = " << x << ", y = " << y << ", score = " << z;

	return z;
}

float calc_score(const vector<float> scores, const float modal_threshold) {
	assert(!scores.empty());

	float score = min(scores[0] / modal_threshold * ABIS_INTEGRA_THRESHOLD, 1.f);
	if (scores.size() > 1)
	{
		for (size_t i = 1; i < scores.size(); i++)
		{
			score = multi_score(score, min(scores[i] / modal_threshold * ABIS_INTEGRA_THRESHOLD, 1.f));
		}
	}
	return score;
}
