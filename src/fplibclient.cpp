#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"

int get_fingerprint_template(unsigned char* image_data, const size_t image_data_len,
	unsigned char* template_buf, const size_t template_buf_len)
{
	int res = 0;
	fp_img* in_fpimg = NULL;

	if (template_buf_len < ABIS_FINGER_TMP_GOST_SIZE) throw runtime_error("Buffer too small.");

	auto ptr = image_data;
	try
	{
		gil::gray8c_view_t img_view;
		convert_image(ptr, image_data_len, img_view);

		in_fpimg = (fp_img*)malloc(sizeof(fp_img) + img_view.width() * img_view.height() + 4);
		in_fpimg->width = img_view.width();
		in_fpimg->height = img_view.height();
		in_fpimg->length = img_view.width() * img_view.height();
		in_fpimg->minutiae = NULL;
		in_fpimg->binarized = NULL;
		in_fpimg->flags = 0;

		unsigned char* fp_data = in_fpimg->data;
		for (auto it = img_view.begin(); it != img_view.end(); ++it) *fp_data++ = *it;

		memset(template_buf, 0, ABIS_FINGER_TMP_GOST_SIZE);

		// only linux implementation
#ifndef _WIN32
		getFingerPrint(in_fpimg, template_buf);
#endif
		res = 1;

	}
	catch (const boost::system::error_code& ec)
	{
		cout << "Exception: get_fingerprint_template " << ec.message() << endl;
		res = -ec.value();
	}
	catch (const std::exception& ec)
	{
		cout << "Exception: get_fingerprint_template " << ec.what() << endl;
		res = -std::error_code().value();
	}

	if (in_fpimg != NULL) free(in_fpimg);
	if (ptr != nullptr) free(ptr);
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
