#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

#define EBS_CMD_EXTRACT_FACE		0x00
#define EBS_CMD_EXTRACT_FINGER		0x01

#define EBS_CHECK_EXTRACT_FACE		0x01
#define EBS_CHECK_EXTRACT_FINGER	0xFE

/*
получает шаблон из изображения лица
*/
int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size);

/*
получает шаблон из изображения пальца
*/
int extract_finger_template(const unsigned char* image_data, const size_t image_data_len,
    void* template_buf, const size_t template_buf_size, bool gost);

int extract_finger_templates(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, void* gost_template_buf, const size_t gost_template_buf_size);
/*
сравнивает два шаблона лица через граф
*/
float cmp_face_tmp(void* tmp1, void* tmp2);

/*
сравнивает два шаблона пальца через граф
*/
float cmp_finger_tmp(void* tmp1, void* tmp2);

#define SW_NORM_P	1000
float sugeno_weber(float x, float y);

#endif
