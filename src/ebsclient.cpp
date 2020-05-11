#include <stdio.h>

#include "AbisRest.h"
#include "ebsclient.h"
#include "imgutils.h"

#define COUNTOF(x) (sizeof(x)/sizeof(*x))

interprocess_semaphore mx_logger(1);
interprocess_semaphore mx_finder(1);
vector<interprocess_semaphore*> mx_ports;

int find_free_port()
{
    if (mx_ports.size() == 0)
        for (size_t i = 0; i < extract_port_count; i++)
        {
            interprocess_semaphore* sem = new interprocess_semaphore(1);
            mx_ports.push_back(sem);
        }
    mx_finder.wait();

    while (true)
    {
        for (size_t i = 0; i < extract_port_count; i++)
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

int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
    void* template_buf, const size_t template_buf_size)
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

        if (err || !client_socket.is_open())
            cout << "extract_face_template: extract service not response. " << err.message() << endl;
        else
        {
            int send_data_len = image_data_len + 1;
            char cmd = 0;

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
            write(client_socket, buffer(image_data, image_data_len), err);
            write(client_socket, buffer(&cmd, 1), err);
            if (!err)
            {
                unsigned char recv_header[8];
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

                        if (recv_data_len > 0)
                        {
                            unsigned char* recv_data = (unsigned char*)malloc(recv_data_len);
                            io_len = client_socket.read_some(buffer(recv_data, recv_data_len), err);
                            if (!err)
                            {
                                if (io_len == recv_data_len && template_buf_size <= recv_data_len - 1)
                                {
                                    memcpy(template_buf, &recv_data[1], template_buf_size);
                                    if (recv_data[0] == 0xFE) res = 1;
                                }
                            }
                            free(recv_data);
                        }
                    }
                }
            }
            client_socket.close();
        }
    }
    catch (const boost::system::error_code& ec)
    {
        cout << "extract_face_template: " << ec.message() << endl;
        res = -1;
    }
    catch (const std::exception& ec)
    {
        cout << "extract_face_template: " << ec.what() << endl;
        res = -2;
    }

    mx_ports[port_index]->post();
    return res;
}

int extract_finger_template(const unsigned char* image_data, const size_t image_data_len,
    void* template_buf, const size_t template_buf_size)
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

        if (err || !client_socket.is_open())
            cout << "extract_face_template: extract service not response. " << err.message() << endl;
        else
        {
            int send_data_len = image_data_len + 1;
            char cmd = 1;

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
            write(client_socket, buffer(image_data, image_data_len), err);
            write(client_socket, buffer(&cmd, 1), err);
            if (!err)
            {
                unsigned char recv_header[8];
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

                        if (recv_data_len > 0)
                        {
                            unsigned char* recv_data = (unsigned char*)malloc(recv_data_len);
                            io_len = client_socket.read_some(buffer(recv_data, recv_data_len), err);
                            if (!err)
                            {
                                if (io_len == recv_data_len && template_buf_size <= recv_data_len - 1)
                                {
                                    memcpy(template_buf, &recv_data[1], template_buf_size);
                                    if (recv_data[0] == 0xFE) res = 1;
                                }
                            }
                            free(recv_data);
                        }
                    }
                }
            }
            client_socket.close();
        }
    }
    catch (const boost::system::error_code& ec)
    {
        cout << "extract_finger_template: " << ec.message() << endl;
        res = -1;
    }
    catch (const std::exception& ec)
    {
        cout << "extract_finger_template: " << ec.what() << endl;
        res = -2;
    }

    mx_ports[port_index]->post();
    return res;
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
