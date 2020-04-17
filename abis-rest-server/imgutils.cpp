#include "imgutils.h"

bool isPng(const unsigned char* img)
{
	if (img == NULL) return false;
	return boost::regex_match(reinterpret_cast<const char*>(img), boost::regex(reinterpret_cast<const char*>(rexpPNG)));
}

bool isJpg(const unsigned char* img)
{
	if (img == NULL) return false;
	return boost::regex_match(reinterpret_cast<const char*>(img), boost::regex(reinterpret_cast<const char*>(rexpJPG)));
}

bool isBmp(const unsigned char* img)
{
	if (img == NULL) return false;
	return boost::regex_match(reinterpret_cast<const char*>(img), boost::regex(reinterpret_cast<const char*>(rexpBMP)));
}
