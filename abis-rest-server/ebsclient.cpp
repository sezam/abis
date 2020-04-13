#include <stdio.h>

#include "AbisRest.h"
#include "ebsclient.h"

int loglevel = log_debug;

interprocess_semaphore mkey1(1);
interprocess_semaphore mx_logger(1);
interprocess_semaphore mkey3(1);
interprocess_semaphore mx_socket(1);
interprocess_semaphore mx_ports[4] = { 1, 1, 1, 1 };

FILE* logFile;

io_service boost_io_service;

char* getLogName(int log_level)
{
	if (log_level == log_error) return (char*)"[ERROR] ";
	if (log_level == log_user) return (char*)"[USER] ";
	if (log_level == log_info) return (char*)"[INFO] ";
	if (log_level == log_debug) return (char*)"[DEBUG] ";
	return (char*)" ";
}

void getTime(char* sTime)
{
	for (int i = 0; i < 64; i++) sTime[i] = 0;
	time_t t = time(NULL);
	struct tm* ltm;
	ltm = localtime(&t);
	char* sTime1 = asctime(ltm);
	for (int i = 0; i < strlen(sTime1); i++) *(sTime + i) = *(sTime1 + i);
	for (int i = 0; i < strlen(sTime1); i++) if (*(sTime + i) == 0x0A)
	{
		*(sTime + i) = 0x0D;
		*(sTime + i + 1) = 0x0A;
	}
}

void saveToLog(int elev, char* log, const char* data)
{
	if (elev <= loglevel)
	{
		mx_logger.wait();
		char* tstr = new char[64];
		getTime(tstr);
		*(tstr + strlen(tstr) - 2) = 0;
		logFile = fopen("../log/gzface.log", "a");
		if (logFile)
		{
			fprintf(logFile, "%s\t%s\t%s %s\n", getLogName(elev), tstr, log, data);
			fclose(logFile);
		}
		delete[] tstr;
		mx_logger.post();
	}
}

int find_free_port()
{
	mx_socket.wait();
	while (true)
	{
		for (size_t i = 0; i < 4; i++)
		{
			int value;
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

int get_face_template(const unsigned char* send_data, const unsigned int send_data_len,
	float* template_buf, const unsigned int template_buf_size)
{
	int rs = 0;

	int port_index = find_free_port();
	int current_port = port_index + 10080;
	saveToLog(log_debug, "Extract from port ", to_string(current_port).c_str());
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

		if (err || !client_socket.is_open())
			throw(err ? err : boost::system::errc::make_error_code(boost::system::errc::connection_aborted));


		char send_header[8];
		send_header[0] = 'r';
		send_header[1] = 'e';
		send_header[2] = 'q';
		send_header[3] = 'f';
		send_header[7] = (unsigned char)(send_data_len & 0xFF);
		send_header[6] = (unsigned char)((send_data_len >> 8) & 0xFF);
		send_header[5] = (unsigned char)((send_data_len >> 16) & 0xFF);
		send_header[4] = (unsigned char)((send_data_len >> 24) & 0xFF);

		write(client_socket, buffer(send_header, 8), err);
		write(client_socket, buffer(send_data, send_data_len), err);
		if (err) {
			client_socket.close();
			throw(err);
		}

		char recv_header[20];
		int io_len = client_socket.read_some(buffer(recv_header), err);
		if (err) {
			client_socket.close();
			throw(err);
		}

		if (recv_header[0] == 'a' && recv_header[1] == 'n' && recv_header[2] == 's' && recv_header[3] == 'w')
		{
			unsigned int p1 = (unsigned int)recv_header[4];
			unsigned int p2 = (unsigned int)recv_header[5];
			unsigned int p3 = (unsigned int)recv_header[6];
			unsigned int p4 = (unsigned int)recv_header[7];
			int recv_data_len = p1 * 16777216 + p2 * 65536 + p3 * 256 + p4;

			char* recv_data = new char[recv_data_len];
			io_len = client_socket.read_some(buffer(recv_data, recv_data_len), err);
			cout << "Recv len: " << io_len << endl;
			if (err) {
				delete[] recv_data;
				client_socket.close();
				throw(err);
			}


			if (io_len == recv_data_len)
			{
				rs = recv_data[0];
				memcpy_s(template_buf, template_buf_size, &recv_data[1], recv_data_len - 1);
				delete[] recv_data;
			}
			else
			{
				delete[] recv_data;
				client_socket.close();
				throw(boost::system::errc::make_error_code(boost::system::errc::bad_message));
			}
		}

		client_socket.close();
	}
	catch (const boost::system::error_code& ec)
	{
		mx_ports[port_index].post();
		cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;
		throw(ec);
	}
	catch (const std::exception& ec)
	{
		mx_ports[port_index].post();
		cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;
		throw(boost::system::errc::make_error_code(boost::system::errc::io_error));
	}

	mx_ports[port_index].post();
	cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;
	return rs;
}

void ebs_client_init()
{
}

void ebs_client_done()
{
}