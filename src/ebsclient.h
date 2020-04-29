#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

#define FACE_TEMPLATE_SIZE	512

/*
расчитывает эвклидово расстояние между векторами
*/
int extract_face_template(const unsigned char* image_data, const size_t image_data_len,
	void* template_buf, const size_t template_buf_size);

/*
сравнивает два шаблона лица 
*/
float cmp_face_tmp(void* tmp1, void* tmp2);

#endif
