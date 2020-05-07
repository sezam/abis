#pragma once

#ifndef IMGUTILS_H
#define IMGUTILS_H

const unsigned char rexpBMP[] = { 0x42, 0x4D };
const unsigned char rexpPNG[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const unsigned char rexpJPG[] = { 0xFF, 0xD8, 0xFF };
const unsigned char rexpWSQ[] = { 0xFF, 0xA6, 0xFF, 0xA8, 0x00, 0x79 };

bool isPNG(const unsigned char* img);
bool isJPG(const unsigned char* img);
bool isBMP(const unsigned char* img);
bool isWSQ(const unsigned char* img);

#endif
