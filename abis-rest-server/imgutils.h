#pragma once

#ifndef IMGUTILS_H
#define IMGUTILS_H

#include "AbisRest.h"

const unsigned char rexpBMP[] = { '^', 0x42, 0x4D };
const unsigned char rexpPNG[] = { '^', 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const unsigned char rexpJPG[] = { '^', 0xFF, 0xD8, 0xFF };

bool isPng(const unsigned char * img);
bool isJpg(const unsigned char* img);
bool isBmp(const unsigned char* img);

float fvec_eq_dis(const float* x, const float* y, size_t d);

#endif
