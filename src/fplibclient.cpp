#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"

int get_fingerprint_template(const unsigned char* image_data, const size_t image_data_len,
	unsigned char* template_buf, const size_t template_buf_len)
{
	int res = 0;
	fp_img* in_fpimg = nullptr;

	assert(image_data != nullptr);
	assert(template_buf != nullptr);
	assert(template_buf_len >= ABIS_FINGER_TMP_GOST_SIZE);

	try
	{
		cv::Mat gr_img = cv::imdecode(cv::Mat(1, image_data_len, CV_8UC1, (char*)image_data), cv::IMREAD_GRAYSCALE);

		size_t img_len = gr_img.rows * gr_img.cols;
		in_fpimg = (fp_img*)malloc(sizeof(fp_img) + img_len + 4);
		in_fpimg->width = gr_img.cols;
		in_fpimg->height = gr_img.rows;
		in_fpimg->length = img_len;
		in_fpimg->minutiae = NULL;
		in_fpimg->binarized = NULL;
		in_fpimg->flags = 0;

		memcpy(in_fpimg->data, gr_img.data, img_len);

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
	return res;
}

float cmp_fingerprint_gost_template(void* tmp1, void* tmp2) {
	float score = 0;
	// only linux implementation
#ifndef _WIN32
	score = matchSegmentsTemplate((unsigned char*)tmp1, 0, 0, (unsigned char*)tmp2, 0, 0) / 100.0f;
#endif
	return score;
}
