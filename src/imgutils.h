#pragma once

#ifndef IMGUTILS_H
#define IMGUTILS_H

#include "wsq.h"

const unsigned char rexpBMP[] = { 0x42, 0x4D };
const unsigned char rexpPNG[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const unsigned char rexpJPG[] = { 0xFF, 0xD8, 0xFF };
const unsigned char rexpWSQ[] = { 0xFF, 0xA0, 0xFF, 0xA8, 0x00, 0x79 };
const unsigned char rexpJP2[] = { 0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A };

bool isPNG(const unsigned char* img);
bool isJPG(const unsigned char* img);
bool isBMP(const unsigned char* img);
bool isWSQ(const unsigned char* img);
bool isJP2(const unsigned char* img);

void convert_image(const unsigned char* i_image_data, const size_t i_image_data_len, unsigned char*& o_image_data, size_t& o_image_data_len);

#endif
