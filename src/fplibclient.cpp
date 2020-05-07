#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"
#include "nbis/wsq.h"

int get_fingerprint_template(const unsigned char* image_data, const size_t image_data_len,
    unsigned char* template_buf, const size_t template_buf_len)
{
    int res = 0;
    vector<void*> garbage;

    if (template_buf_len < FINGER_TEMPLATE_SIZE) throw runtime_error("Buffer too small.");

    try
    {
        gil::gray8c_view_t img_view;
        gil::gray8_image_t img;
        std::stringstream inbuf;

        bool prepare_img = false;

        if (isBMP(image_data))
        {
            inbuf.write((const char*)image_data, image_data_len);
            gil::read_and_convert_image(inbuf, img, gil::bmp_tag());
            img_view = gil::const_view(img);

            prepare_img = true;
        }
        if (isPNG(image_data))
        {
            inbuf.write((const char*)image_data, image_data_len);
            gil::read_and_convert_image(inbuf, img, gil::png_tag());
            img_view = gil::const_view(img);

            prepare_img = true;
        }
        if (isJPG(image_data))
        {
            inbuf.write((const char*)image_data, image_data_len);
            gil::read_and_convert_image(inbuf, img, gil::jpeg_tag());
            img_view = gil::const_view(img);

            prepare_img = true;
        }
        if (isWSQ(image_data))
        {
            unsigned char* img_ptr;
            int width, height, depth, ppi, lossy;
            int res = wsq_decode_mem(&img_ptr, &width, &height, &depth, &ppi, &lossy,
                (unsigned char*)image_data, image_data_len);
            garbage.push_back(img_ptr);

            img_view = gil::interleaved_view(width, height, (gil::gray8_pixel_t*) img_ptr, width);
            prepare_img = true;
        }

        if (!prepare_img) throw runtime_error("Unknown image format");

        fp_img* in_fpimg = (fp_img*)malloc(sizeof(fp_img) + img.width() * img.height());
        if (in_fpimg == NULL) throw runtime_error("Memory allocate problem");
        garbage.push_back(in_fpimg);
        in_fpimg->width = img.width();
        in_fpimg->height = img.height();
        in_fpimg->length = img.width() * img.height();
        in_fpimg->minutiae = NULL;
        in_fpimg->binarized = NULL;
        in_fpimg->flags = 0;

        unsigned char* fp_data = in_fpimg->data;
        for (auto it = img_view.begin(); it != img_view.end(); ++it) *fp_data++ = *it;

#ifdef _WIN32
        for (size_t i = 0; i < FINGER_TEMPLATE_SIZE; i++)
        {
            template_buf[i] = i % 255;
        }
#else
        // ðåàëèçàöèÿ åñòü òîëüêî ïîä linux
        getFingerPrint(in_fpimg, template_buf);
#endif
        res = *((int*)template_buf);

    }
    catch (const boost::system::error_code& ec)
    {
        res = -1;
        cout << "get_fingerprint_template: " << ec.message() << endl;
    }
    catch (const std::exception& ec)
    {
        res = -1;
        cout << "get_fingerprint_template: " << ec.what() << endl;
    }

    for (auto m : garbage) free(m);
    return res;
}

float cmp_fingerprint_tmp(void* tmp1, void* tmp2) {
    float score = 0;
#ifdef _WIN32
    score = rand() % 100 / 100.0f;
#else
    // ðåàëèçàöèÿ åñòü òîëüêî ïîä linux
    score = matchFingerPrint((unsigned char*)tmp1, (unsigned char*)tmp2) / 100.0f;
#endif
    return score;
}
