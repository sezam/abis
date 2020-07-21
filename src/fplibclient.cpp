#include "AbisRest.h"
#include "fplibclient.h"
#include "imgutils.h"

int get_fingerprint_template(const unsigned char* image_data, const size_t image_data_len,
	unsigned char* template_buf, const size_t template_buf_len)
{
	assert(image_data != nullptr);
	assert(template_buf != nullptr);
	assert(template_buf_len >= ABIS_FINGER_TMP_GOST_SIZE);

	int res = 0;
	fp_img* in_fpimg = nullptr;

	try
	{
		cv::Mat in_img = cv::Mat(1, image_data_len, CV_8UC1, (char*)image_data);
		cv::Mat gr_img = cv::imdecode(in_img, cv::IMREAD_GRAYSCALE);

		size_t img_len = gr_img.rows * gr_img.cols;
		in_fpimg = (fp_img*)malloc(sizeof(fp_img) + img_len + 4);
		in_fpimg->width = gr_img.cols;
		in_fpimg->height = gr_img.rows;
		in_fpimg->length = img_len;
		in_fpimg->minutiae = nullptr;
		in_fpimg->binarized = nullptr;
		in_fpimg->flags = 0;

		memcpy(in_fpimg->data, gr_img.data, img_len);
		in_img.release();
		gr_img.release();

		memset(template_buf, 0, ABIS_FINGER_TMP_GOST_SIZE);

		// only linux implementation
#ifndef _WIN32
		getFingerPrint(in_fpimg, template_buf);
#endif
		int q = 0;
		xyt_struct* xyt = (xyt_struct*)template_buf;
		if (xyt->nrows >= finger_min_points)
		{
			for (size_t i = 0; i < xyt->nrows; i++) q += (int)(xyt->quality[i] >= finger_min_quality);
			res = (int)(q >= xyt->nrows * finger_min_goodpoints / 100);
		}
		BOOST_LOG_TRIVIAL(debug) << "get_fingerprint_template: points = " << xyt->nrows << " good = " << q << " res = " << res;
	}
	catch (const boost::system::error_code& ec)
	{
		BOOST_LOG_TRIVIAL(error) << "Exception: get_fingerprint_template " << ec.message();
		res = -ec.value();
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(error) << "Exception: get_fingerprint_template " << ec.what();
		res = -std::error_code().value();
	}

	if (in_fpimg != nullptr) free(in_fpimg);
	return res;
}

float cmp_fingerprint_gost_template(void* tmp1, void* tmp2) {
	assert(tmp1 != nullptr);
	assert(tmp2 != nullptr);

	float score = 0.0123456;
	// only linux implementation
#ifndef _WIN32		
	score = matchSegmentsTemplateArea((unsigned char*)tmp1, 0, 0, (unsigned char*)tmp2, 0, 0) / 100.0f;
#endif
	BOOST_LOG_TRIVIAL(debug) << "cmp_fingerprint_gost_template: score = " << score;
	return score;
}
