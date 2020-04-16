#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"

int get_fingerprint_template(const unsigned char* image_data, const unsigned int image_data_len, unsigned char* template_buf)
{
	fp_img* in_fpimg = NULL;

	try
	{
		gil::gray8_image_t img;

		std::stringstream inbuf;
		inbuf.write((const char*)image_data, image_data_len);

		if (isBmp(image_data))
		{
			gil::read_and_convert_image(inbuf, img, gil::bmp_tag());
		}
		if (isPng(image_data))
		{
			gil::read_and_convert_image(inbuf, img, gil::png_tag());
		}
		if (isJpg(image_data))
		{
			gil::read_and_convert_image(inbuf, img, gil::jpeg_tag());
		}

		in_fpimg = (fp_img*)malloc(sizeof(fp_img) + img.width() * img.height() + 4);
		in_fpimg->width = img.width();
		in_fpimg->height = img.height();
		in_fpimg->length = img.width() * img.height();
		in_fpimg->minutiae = NULL;
		in_fpimg->binarized = NULL;
		in_fpimg->flags = 0;

		auto img_view = gil::const_view(img);
		unsigned char* fp_data = in_fpimg->data;
		for (auto it = img_view.begin(); it != img_view.end(); ++it) fp_data++[*it];

		memset(template_buf, 0, sizeof(xyt_struct));

		// реализация есть только под linux
#ifdef _WIN32
		for (size_t i = 0; i < FINGER_TEMPLATE_SIZE; i++)
		{
			template_buf[i] = i % 255;
		}
#else
		getFingerPrint(in_fpimg, template_buf);
#endif
	}
	catch (const boost::system::error_code& ec)
	{
		if (in_fpimg != NULL) free(in_fpimg);
	}
	catch (const std::exception& ec)
	{
		if (in_fpimg != NULL) free(in_fpimg);
	}

	if (in_fpimg != NULL) free(in_fpimg);
	return 0;
}

