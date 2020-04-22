#include "imgutils.h"

bool isPng(const unsigned char* img)
{
    for (size_t i = 0; i < std::size(rexpPNG); i++)
    {
        if (img[i] != rexpBMP[i]) return false;
    }
    return true;
}

bool isJpg(const unsigned char* img)
{
    for (size_t i = 0; i < std::size(rexpJPG); i++)
    {
        if (img[i] != rexpBMP[i]) return false;
    }
    return true;
}

bool isBmp(const unsigned char* img)
{
    for (size_t i = 0; i < std::size(rexpBMP); i++)
    {
        if (img[i] != rexpBMP[i]) return false;
    }
    return true;
}

