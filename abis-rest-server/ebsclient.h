#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

#define FACE_TEMPLATE_SIZE	512

int get_face_template(const unsigned char* image_data, const unsigned int image_data_len,
	void* template_buf, const unsigned int template_buf_size);

#endif
