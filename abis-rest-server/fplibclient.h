#pragma once

#ifndef FPLIBCLIENT_H
#define FPLIBCLIENT_H

#include "finger/libfprint.h"

#define MAX_BOZORTH_MINUTIAE		200
#define MIN_BOZORTH_MINUTIAE		0

struct xyt_struct {
	int nrows;
	int xcol[MAX_BOZORTH_MINUTIAE];
	int ycol[MAX_BOZORTH_MINUTIAE];
	int thetacol[MAX_BOZORTH_MINUTIAE];
	int quality[MAX_BOZORTH_MINUTIAE];
};

#define FINGER_TEMPLATE_SIZE sizeof(xyt_struct)

int get_fingerprint_template(const unsigned char* image_data, const unsigned int image_data_len, 
	unsigned char* template_buf, const unsigned int template_buf_len);

#endif
