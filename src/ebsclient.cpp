#include <stdio.h>

#include "AbisRest.h"
#include "ebsclient.h"

interprocess_semaphore mx_logger(1);
interprocess_semaphore mx_finder(1);
interprocess_semaphore mx_ports[4] = { {1}, {1}, {1}, {1} };

io_service boost_io_service;

int find_free_port()
{
    mx_finder.wait();

    while (true)
    {
        for (size_t i = 0; i < 4; i++)
        {
            if (mx_ports[i].try_wait())
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

int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
    void* template_buf, const size_t template_buf_size)
{
    int port_index = find_free_port();
    if (port_index < 0) return -1;

    int res = 0;
    int current_port = port_index + 10080;
    try
    {
        tcp::socket client_socket(boost_io_service);
        tcp::resolver::query query("10.6.46.147", to_string(current_port));
        tcp::resolver::iterator endpoint_iterator = tcp::resolver(boost_io_service).resolve(query);

        boost::system::error_code err = boost::asio::error::would_block;
        connect(client_socket, endpoint_iterator, err);

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
                char recv_header[8];
                size_t io_len = client_socket.read_some(buffer(recv_header, 8), err);
                if (!err)
                {
                    if (recv_header[0] == 'a' && recv_header[1] == 'n' && recv_header[2] == 's' && recv_header[3] == 'w')
                    {
                        unsigned char p1 = recv_header[4];
                        unsigned char p2 = recv_header[5];
                        unsigned char p3 = recv_header[6];
                        unsigned char p4 = recv_header[7];
                        size_t recv_data_len = (size_t)(p1 * 16777216 + p2 * 65536 + p3 * 256 + p4);

                        char* recv_data = (char*)malloc(recv_data_len);
                        io_len = client_socket.read_some(buffer(recv_data, recv_data_len), err);
                        if (!err)
                        {
                            if (io_len == recv_data_len && template_buf_size == recv_data_len - 1)
                            {
                                res = recv_data[0];
                                memcpy(template_buf, &recv_data[1], recv_data_len);
                            }
                        }
                        free(recv_data);
                    }
                }
            }
            client_socket.close();
        }
        else cout << "extract_face_template: extract service not response. " << err.message() << endl;
    }
    catch (const std::exception& ec)
    {
        res = std::error_code().value();
        cout << "Exception: " << ec.what() << endl;
        res = -1;
    }

    mx_ports[port_index].post();
    return res;
}

float fvec_eq_dis(const float* x, const float* y, size_t size)
{
    float res = 0;

    for (size_t i = 0; i < size; i++) {
        const float tmp = x[i] - y[i];
        const float r = tmp * tmp;
        res += r;
    }
    return sqrt(res);
}

float cmp_face_tmp(void* tmp1, void* tmp2) {
    return 1.0f - fvec_eq_dis((const float*)tmp1, (const float*)tmp2, FACE_TEMPLATE_SIZE);
}
