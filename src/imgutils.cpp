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

bool isJP2(const unsigned char* img)
{
	for (size_t i = 0; i < sizeof(rexpJP2); i++)
	{
		if (img[i] != rexpJP2[i]) return false;
	}
	return true;
}

void convert_image(const unsigned char* i_image_data, const size_t i_image_data_len, unsigned char*& o_image_data, size_t& o_image_data_len)
{
	cv::Mat gr_img;
	unsigned char* img_ptr = nullptr;

	if (isWSQ(i_image_data))
	{
		int width, height, depth, ppi, lossy;

		int res = wsq_decode_mem(&img_ptr, &width, &height, &depth, &ppi, &lossy, (unsigned char*)i_image_data, i_image_data_len);
		if (res) throw runtime_error("Error convert wsq");

		gr_img = cv::Mat(height, width, CV_8UC1, (char*)img_ptr);
	}
	else
	{
		gr_img = cv::imdecode(cv::Mat(1, i_image_data_len, CV_8UC1, (char*)i_image_data), cv::IMREAD_GRAYSCALE);
	}

	if (gr_img.data == nullptr) throw runtime_error("Unsupported image format");

	vector<uchar> out_img;
	vector<int> out_params{ 0 };
	cv::imencode(".png", gr_img, out_img, out_params);

	o_image_data_len = out_img.size();
	o_image_data = (unsigned char*)malloc(o_image_data_len);
	memcpy(o_image_data, out_img.data(), o_image_data_len);

	cv::imwrite("last_finger_image.png", gr_img);
	gr_img.release();
	//if (img_ptr != nullptr) free(img_ptr);
}
