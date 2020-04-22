#include "imgutils.h"

bool isPng(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpPNG); i++)
    {
        if (img[i] != rexpPNG[i]) return false;
    }
    return true;
}

bool isJpg(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpJPG); i++)
    {
        if (img[i] != rexpJPG[i]) return false;
    }
    return true;
}

bool isBmp(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpBMP); i++)
    {
        if (img[i] != rexpBMP[i]) return false;
    }
    return true;
}

