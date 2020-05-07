#include "imgutils.h"

bool isPNG(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpPNG); i++)
    {
        if (img[i] != rexpPNG[i]) return false;
    }
    return true;
}

bool isJPG(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpJPG); i++)
    {
        if (img[i] != rexpJPG[i]) return false;
    }
    return true;
}

bool isBMP(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpBMP); i++)
    {
        if (img[i] != rexpBMP[i]) return false;
    }
    return true;
}

bool isWSQ(const unsigned char* img)
{
    for (size_t i = 0; i < sizeof(rexpWSQ); i++)
    {
        if (img[i] != rexpWSQ[i]) return false;
    }
    return true;
}
