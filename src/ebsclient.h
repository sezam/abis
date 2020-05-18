#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

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
