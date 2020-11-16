#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

#define EBS_CMD_EXTRACT_FACE		0x00
#define EBS_CMD_EXTRACT_FINGER		0x01
#define EBS_CMD_EXTRACT_FINGER_XYT	0x02
#define EBS_CMD_EXTRACT_IRIS		0x03

#define EBS_CHECK_EXTRACT_FACE		0x01
#define EBS_CHECK_EXTRACT_FINGER	0xFE
#define EBS_CHECK_EXTRACT_IRIS		0x01

/*
получает шаблон из изображения лица
*/
int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size);

/*
получает шаблон из изображения пальца
*/
int extract_finger_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, const bool gost);

int extract_finger_templates(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, void* gost_template_buf, const size_t gost_template_buf_size);

int extract_finger_xyt(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size, void* gost_template_buf, const size_t gost_template_buf_size);

/*
получает шаблон из радужки
*/
int extract_iris_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size);

/*
сравнивает два шаблона лица через эвклидово расстояние
*/
float cmp_face_tmp(const void* tmp1, const void* tmp2);

/*
сравнивает два шаблона пальца через эвклидово расстояние
*/
float cmp_finger_tmp(const void* tmp1, const void* tmp2);

/*
сравнивает два шаблона радужки через эвклидово расстояние
*/
float cmp_iris_tmp(const void* tmp1, const void* tmp2);

#define SW_NORM_P	1000
/*
расчет комплексной оценки по двум оценкам
*/
float sugeno_weber(const float x, const float y);

/*
расчет комплексной оценки по двум оценкам
*/
float multi_score(const float x, const float y);

float calc_score(const vector<float> scores, const float modal_threshold);

#endif
