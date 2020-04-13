#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

#define FACESIZE 512

void saveToLog(int elev, char *log, const char *data);

int get_face_template(const unsigned char* send_data, const unsigned int send_data_len,
	float* template_buf, const unsigned int template_buf_size);

void ebs_client_init();

void ebs_client_done();

#endif
