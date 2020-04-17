#include "imgutils.h"

bool isPng(const unsigned char* img)
{
	return boost::regex_match(reinterpret_cast<const char*>(img), boost::regex(reinterpret_cast<const char*>(rexpPNG)));
}

bool isJpg(const unsigned char* img)
{
	return boost::regex_match(reinterpret_cast<const char*>(img), boost::regex(reinterpret_cast<const char*>(rexpJPG)));
}

bool isBmp(const unsigned char* img)
{
	return boost::regex_match(reinterpret_cast<const char*>(img), boost::regex(reinterpret_cast<const char*>(rexpBMP)));
}

float fvec_eq_dis(const float* x, const float* y, size_t d)
{
	size_t i;
	float res;

	res = (x[0] - y[0]);
	for (i = 1; i < d; i++) {
		const float tmp = x[i] - y[i];
		res += tmp * tmp;
	}
	return sqrt(res);
}
