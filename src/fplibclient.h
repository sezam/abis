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

#define ABIS_FINGER_TMP_GOST_LEN    sizeof(xyt_struct)
#define ABIS_FINGER_TMP_GOST_SIZE   ABIS_FINGER_TMP_GOST_LEN


int get_fingerprint_template(unsigned char* image_data, const size_t image_data_len,
	unsigned char* template_buf, const size_t template_buf_len);

float cmp_fingerprint_gost_template(void* tmp1, void* tmp2);

#endif
