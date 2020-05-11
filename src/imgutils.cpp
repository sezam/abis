#include "AbisRest.h"
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

void convert_image(const unsigned char* i_image_data, const size_t i_image_data_len,
    unsigned char*& o_image_data, const size_t& o_image_data_len)
{
    gil::gray8c_view_t img_view;
    gil::gray8_image_t img;
    std::stringstream inbuf;

    bool prepare_img = false;

    if (isBMP(i_image_data))
    {
        inbuf.write((const char*)i_image_data, i_image_data_len);
        gil::read_and_convert_image(inbuf, img, gil::bmp_tag());
        img_view = gil::const_view(img);

        prepare_img = true;
    }
    if (isPNG(i_image_data))
    {
        inbuf.write((const char*)i_image_data, i_image_data_len);
        gil::read_and_convert_image(inbuf, img, gil::png_tag());
        img_view = gil::const_view(img);

        prepare_img = true;
    }
    if (isJPG(i_image_data))
    {
        inbuf.write((const char*)i_image_data, i_image_data_len);
        gil::read_and_convert_image(inbuf, img, gil::jpeg_tag());
        img_view = gil::const_view(img);

        prepare_img = true;
    }
    if (isWSQ(i_image_data))
    {
        unsigned char* img_ptr;
        int width, height, depth, ppi, lossy;
        int res = wsq_decode_mem(&img_ptr, &width, &height, &depth, &ppi, &lossy,
            (unsigned char*)i_image_data, i_image_data_len);

        img_view = gil::interleaved_view(width, height, (gil::gray8_pixel_t*) img_ptr, width);
        prepare_img = true;
    }

    if (!prepare_img) throw runtime_error("Unknown image format");
}
