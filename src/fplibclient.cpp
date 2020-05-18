#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"

int get_fingerprint_template(const unsigned char* image_data, const size_t image_data_len,
	unsigned char* template_buf, const size_t template_buf_len)
{
	int res = -1;
	fp_img* in_fpimg = NULL;

	if (template_buf_len < ABIS_FINGER_TMP_GOST_SIZE) throw runtime_error("Buffer too small.");

	try
	{
		gil::gray8_image_t img;
		bool prepare_img = false;

		std::stringstream inbuf;
		inbuf.write((const char*)image_data, image_data_len);

		if (isBMP(image_data))
		{
			gil::read_and_convert_image(inbuf, img, gil::bmp_tag());
			prepare_img = true;
		}
		if (isPNG(image_data))
		{
			gil::read_and_convert_image(inbuf, img, gil::png_tag());
			prepare_img = true;
		}
		if (isJPG(image_data))
		{
			gil::read_and_convert_image(inbuf, img, gil::jpeg_tag());
			prepare_img = true;
		}

		if (prepare_img)
		{
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

			memset(template_buf, 0, ABIS_FINGER_TMP_GOST_SIZE);

			// only linux implementation
#ifndef _WIN32
			getFingerPrint(in_fpimg, template_buf);
#endif
			res = 0;
		}
	}
	catch (const boost::system::error_code& ec)
	{
		if (in_fpimg != NULL) free(in_fpimg);
		res = ec.value();
	}
	catch (const std::exception& ec)
	{
		if (in_fpimg != NULL) free(in_fpimg);
		res = std::error_code().value();
	}

	if (in_fpimg != NULL) free(in_fpimg);
	return res;
}

float cmp_fingerprint_gost_template(void* tmp1, void* tmp2) {
	float score = 0;
	// only linux implementation
#ifndef _WIN32
	score = matchFingerPrint((unsigned char*)tmp1, (unsigned char*)tmp2) / 100.0f;
#endif
	return score;
}
