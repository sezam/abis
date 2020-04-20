#include <stdio.h>

#include "AbisRest.h"
#include "ebsclient.h"

interprocess_semaphore mx_logger(1);
interprocess_semaphore mx_socket(1);
interprocess_semaphore mx_ports[4] = { {1}, {1}, {1}, {1} };

io_service boost_io_service;

int find_free_port()
{
	mx_socket.wait();
	while (true)
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (mx_ports[i].try_wait()) {
				mx_socket.post();
				return i;
			}
		}
		this_thread::sleep_for(milliseconds(100));
	};

	mx_socket.post();
	return -1;
}

int get_face_template(const unsigned char* image_data, const unsigned int image_data_len,
	void* template_buf, const unsigned int template_buf_size)
{
	int res = -1;

	int port_index = find_free_port();
	int current_port = port_index + 10080;
	cout << "Mutex lock: " << &mx_ports[port_index] << " port index: " << port_index << endl;

	try
	{
		tcp::socket client_socket(boost_io_service);
		tcp::resolver::query query("10.6.46.147", to_string(current_port));
		tcp::resolver::iterator endpoint_iterator = tcp::resolver(boost_io_service).resolve(query);

		boost::system::error_code err = boost::asio::error::would_block;
		deadline_timer deadline(boost_io_service, boost::posix_time::seconds(3));

		async_connect(client_socket, endpoint_iterator, var(err) = _1);
		do boost_io_service.run_one(); while (err == boost::asio::error::would_block);

		if (!err || client_socket.is_open())
		{
			char send_header[8];
			send_header[0] = 'r';
			send_header[1] = 'e';
			send_header[2] = 'q';
			send_header[3] = 'f';
			send_header[7] = (unsigned char)(image_data_len & 0xFF);
			send_header[6] = (unsigned char)((image_data_len >> 8) & 0xFF);
			send_header[5] = (unsigned char)((image_data_len >> 16) & 0xFF);
			send_header[4] = (unsigned char)((image_data_len >> 24) & 0xFF);

			write(client_socket, buffer(send_header, 8), err);
			write(client_socket, buffer(image_data, image_data_len), err);
			if (!err)
			{
				char recv_header[20];
				int io_len = client_socket.read_some(buffer(recv_header), err);
				if (!err)
				{
					if (recv_header[0] == 'a' && recv_header[1] == 'n' && recv_header[2] == 's' && recv_header[3] == 'w')
					{
						unsigned int p1 = (unsigned int)recv_header[4];
						unsigned int p2 = (unsigned int)recv_header[5];
						unsigned int p3 = (unsigned int)recv_header[6];
						unsigned int p4 = (unsigned int)recv_header[7];
						int recv_data_len = p1 * 16777216 + p2 * 65536 + p3 * 256 + p4;

						char* recv_data = new char[recv_data_len];
						io_len = client_socket.read_some(buffer(recv_data, recv_data_len), err);
						if (!err)
						{
							if (io_len == recv_data_len && template_buf_size == recv_data_len - 1)
							{
								res = recv_data[0];
								memcpy(template_buf, &recv_data[1], recv_data_len);
							}
						}
						delete[] recv_data;
					}
				}
			}
			client_socket.close();
		}
	}
	catch (const std::exception& ec)
	{
		res = std::error_code().value();
	}

	mx_ports[port_index].post();
	cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;
	return res;
}

float fvec_eq_dis(const float* x, const float* y, size_t d)
{
	size_t i;
	float res;

	res = (x[0] - y[0]);
	for (i = 1; i < d; i++) {
		const float tmp = x[i] - y[i];
		res += tmp * tmp;
	}
	return sqrt(res);
}

float cmp_face_template(void* tmp1, void* tmp2) {
	return fvec_eq_dis((const float*)tmp1, (const float*)tmp1, FACE_TEMPLATE_SIZE);
}
