﻿#include <stdio.h>

#include "AbisRest.h"
#include "ebsclient.h"
#include "imgutils.h"
#include "fplibclient.h"

#define COUNTOF(x) (sizeof(x)/sizeof(*x))

interprocess_semaphore mx_logger(1);
interprocess_semaphore mx_finder(1);
vector<interprocess_semaphore*> mx_ports;

int find_free_port()
{
	if (mx_ports.size() == 0)
		for (int i = 0; i < extract_port_count; i++)
		{
			interprocess_semaphore* sem = new interprocess_semaphore(1);
			mx_ports.push_back(sem);
		}
	mx_finder.wait();

	while (true)
	{
		for (int i = 0; i < extract_port_count; i++)
		{
			if (mx_ports[i]->try_wait())
			{
				mx_finder.post();
				return i;
			}
			this_thread::yield();
			cout << "extract face service busy" << endl;
		}
		this_thread::sleep_for(milliseconds(100));
	};

	mx_finder.post();
	return -1;
}

int ebs_request(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, const char cmd, const char check, const size_t offset)
{
	int port_index = find_free_port();
	if (port_index < 0) return -1;

	int res = 0;
	int current_port = port_index + extract_port_start;
	try
	{
		io_service boost_io_service;
		tcp::socket client_socket(boost_io_service);
		tcp::resolver::query query(extract_host, to_string(current_port));
		tcp::resolver::iterator endpoint_iterator = tcp::resolver(boost_io_service).resolve(query);

		boost::system::error_code err = boost::asio::error::would_block;
		connect(client_socket, endpoint_iterator, err);

		bool step = !err.failed() && client_socket.is_open();
		if (!step) cout << "ebs_request: extract service not response. " << err.message() << endl;

		int send_data_len = image_data_len + 1;
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

			write(client_socket, buffer(send_header, 8), err);
			step = !err.failed();
			if (!step) cout << "ebs_request: send header error. " << err.message() << endl;
		}

		if (step)
		{
			write(client_socket, buffer(image_data, image_data_len), err);
			step = !err.failed();
			if (!step) cout << "ebs_request: send image error. " << err.message() << endl;
		}

		if (step)
		{
			write(client_socket, buffer(&cmd, 1), err);
			step = !err.failed();
			if (!step) cout << "ebs_request: send cmd error. " << err.message() << endl;
		}

		unsigned char recv_header[8];
		if (step)
		{
			size_t io_len = client_socket.read_some(buffer(recv_header, 8), err);
			step = !err.failed() && io_len == 8;
			if (!step) cout << "ebs_request: receive header error. " << err.message() << endl;
		}

		if (step)
		{
			step = recv_header[0] == 'a' && recv_header[1] == 'n' && recv_header[2] == 's' && recv_header[3] == 'w';
			if (!step) cout << "ebs_request: receive header format error. " << err.message() << endl;
		}

		size_t recv_data_len = 0;
		if (step)
		{
			unsigned char p1 = recv_header[4];
			unsigned char p2 = recv_header[5];
			unsigned char p3 = recv_header[6];
			unsigned char p4 = recv_header[7];
			recv_data_len = (size_t)(p1 * 16777216 + p2 * 65536 + p3 * 256 + p4);

			step = recv_data_len > 0;
			if (!step) cout << "ebs_request: receive header datalen error. " << err.message() << endl;
		}

		unsigned char* recv_data = nullptr;
		if (step)
		{
			recv_data = (unsigned char*)malloc(recv_data_len);
			step = recv_data != nullptr;
			if (!step) cout << "ebs_request: receive data allocate memory error. " << err.message() << endl;
		}

		if (step)
		{
			memset(recv_data, 0, recv_data_len);
			int io_len = client_socket.read_some(buffer(recv_data, recv_data_len), err);

			step = !err.failed() && io_len == recv_data_len && template_buf_size <= recv_data_len - 1;
			if (!step) cout << "ebs_request: receive data allocate memory error. " << err.message() << endl;
		}

		if (step)
		{
			memcpy(template_buf, &recv_data[offset], template_buf_size);
			step = recv_data[0] == check;
		}
		if (recv_data != nullptr) free(recv_data);

		client_socket.close();
		(step ? res = 1 : 0);
	}
	catch (const boost::system::error_code& ec)
	{
		cout << "ebs_request: " << ec.message() << endl;
		res = -1;
	}
	catch (const std::exception& ec)
	{
		cout << "ebs_request: " << ec.what() << endl;
		res = -2;
	}

	mx_ports[port_index]->post();
	return res;

}

int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size)
{
	return ebs_request(image_data, image_data_len, template_buf, template_buf_size, 0, 1, 1);
}

int extract_finger_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, bool gost)
{

	return ebs_request(image_data, image_data_len, template_buf, template_buf_size, 1, 0xFE, (gost ? ABIS_FINGER_TMP_GOST_SIZE : 1));
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

float cmp_face_tmp(void* tmp1, void* tmp2) {
	return 1.0f - min(fvec_eq_dis((const float*)tmp1, (const float*)tmp2, ABIS_TEMPLATE_LEN), 2.0f) / 2.0f;
}

float cmp_finger_tmp(void* tmp1, void* tmp2) {
	return 1.0f - min(fvec_eq_dis((const float*)tmp1, (const float*)tmp2, ABIS_TEMPLATE_LEN), 2.0f) / 2.0f;
}

float sugeno_weber(float x, float y)
{
	return max((x + y + x * y * SW_NORM_P - 1) / (SW_NORM_P + 1), 0.0f);
}