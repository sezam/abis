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

void convert_image(unsigned char*& image_data, size_t image_data_len, gil::gray8c_view_t& img_view)
{
	gil::gray8_image_t img;
	std::stringstream inbuf;

	bool prepare_img = false;

	if (isBMP(image_data))
	{
		inbuf.write((const char*)image_data, image_data_len);
		gil::read_and_convert_image(inbuf, img, gil::bmp_tag());
		img_view = gil::const_view(img);

		prepare_img = true;
		image_data = nullptr;
	}
	if (isPNG(image_data))
	{
		inbuf.write((const char*)image_data, image_data_len);
		gil::read_and_convert_image(inbuf, img, gil::png_tag());
		img_view = gil::const_view(img);

		prepare_img = true;
		image_data = nullptr;
	}
	if (isJPG(image_data))
	{
		inbuf.write((const char*)image_data, image_data_len);
		gil::read_and_convert_image(inbuf, img, gil::jpeg_tag());
		img_view = gil::const_view(img);

		prepare_img = true;
		image_data = nullptr;
	}
	if (isWSQ(image_data))
	{
		unsigned char* img_ptr;
		int width, height, depth, ppi, lossy;
		int res = wsq_decode_mem(&img_ptr, &width, &height, &depth, &ppi, &lossy, (unsigned char*)image_data, image_data_len);
		if (res) throw runtime_error("Error convert wsq");

		image_data = img_ptr;

		img_view = gil::interleaved_view(width, height, (gil::gray8_pixel_t*) img_ptr, width);
		// img_view = gil::interleaved_view(width, height, reinterpret_cast<const boost::gil::gray8_image_t*>(img_ptr), width);
		prepare_img = true;
	}

	if (!prepare_img) throw runtime_error("Unknown image format");
}
