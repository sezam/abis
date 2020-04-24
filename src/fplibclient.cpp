#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"

int get_fingerprint_template(const unsigned char* image_data, const size_t image_data_len,
    unsigned char* template_buf, const size_t template_buf_len)
{
    int res = 0;
    fp_img* in_fpimg = NULL;

    if (template_buf_len < FINGER_TEMPLATE_SIZE) throw runtime_error("Buffer too small.");

    try
    {
        gil::gray8_image_t img;
        bool prepare_img = false;

        std::stringstream inbuf;
        inbuf.write((const char*)image_data, image_data_len);

        if (isBmp(image_data))
        {
            gil::read_and_convert_image(inbuf, img, gil::bmp_tag());
            prepare_img = true;
        }
        if (isPng(image_data))
        {
            gil::read_and_convert_image(inbuf, img, gil::png_tag());
            prepare_img = true;
        }
        if (isJpg(image_data))
        {
            gil::read_and_convert_image(inbuf, img, gil::jpeg_tag());
            prepare_img = true;
        }

        if (!prepare_img) throw runtime_error("Unknown image format");

        in_fpimg = (fp_img*)malloc(sizeof(fp_img) + img.width() * img.height() + 4);
        in_fpimg->width = img.width();
        in_fpimg->height = img.height();
        in_fpimg->length = img.width() * img.height();
        in_fpimg->minutiae = NULL;
        in_fpimg->binarized = NULL;
        in_fpimg->flags = 0;

        auto img_view = gil::const_view(img);
        unsigned char* fp_data = in_fpimg->data;
        for (auto it = img_view.begin(); it != img_view.end(); ++it) *fp_data++ = *it;

#ifdef _WIN32
        for (size_t i = 0; i < FINGER_TEMPLATE_SIZE; i++)
        {
            template_buf[i] = i % 255;
        }
#else
        // реализация есть только под linux
        getFingerPrint(in_fpimg, template_buf);
#endif
        res = *((int*)template_buf);

    }
    catch (const boost::system::error_code& ec)
    {
        if (in_fpimg != NULL) free(in_fpimg);
        //res = ec.value();
        res = -1;
    }
    catch (const std::exception& ec)
    {
        if (in_fpimg != NULL) free(in_fpimg);
        //res = std::error_code().value();
        res = -1;
    }

    if (in_fpimg != NULL) free(in_fpimg);
    return res;
}

float cmp_fingerprint_template(void* tmp1, void* tmp2) {
    float score = 0;
#ifdef _WIN32
    score = rand() % 100 / 100.0f;
#else
    // реализация есть только под linux
    score = matchFingerPrint((unsigned char*)tmp1, (unsigned char*)tmp2) / 100.0f;
#endif
    return score;
}
