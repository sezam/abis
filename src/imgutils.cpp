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
	gil::gray8_view_t img_view;
	gil::gray8_image_t iim;
	unsigned char* img_ptr = nullptr;

	bool prepare_img = false;
	std::stringstream inbuf(ios_base::in | ios_base::out | ios_base::binary);

	if (isBMP(i_image_data))
	{
		inbuf.write((const char*)i_image_data, i_image_data_len);
		gil::read_and_convert_image(inbuf, iim, gil::bmp_tag());
		img_view = gil::view(iim);

		prepare_img = true;
	}
	if (isPNG(i_image_data))
	{
		inbuf.write((const char*)i_image_data, i_image_data_len);
		gil::read_and_convert_image(inbuf, iim, gil::png_tag());
		img_view = gil::view(iim);

		prepare_img = true;
	}
	if (isJPG(i_image_data))
	{
		inbuf.write((const char*)i_image_data, i_image_data_len);
		gil::read_and_convert_image(inbuf, iim, gil::jpeg_tag());
		img_view = gil::view(iim);

		prepare_img = true;
	}
	if (isWSQ(i_image_data))
	{
		int width, height, depth, ppi, lossy;
		int res = wsq_decode_mem(&img_ptr, &width, &height, &depth, &ppi, &lossy, (unsigned char*)i_image_data, i_image_data_len);
		if (res) throw runtime_error("Error convert wsq");

		img_view = gil::interleaved_view(width, height, (gil::gray8_pixel_t*) img_ptr, width);
		prepare_img = true;
	}

	if (prepare_img)
	{
		inbuf.str("");
		gil::write_view(inbuf, img_view, gil::png_tag());

		o_image_data_len = inbuf.tellp();
		o_image_data = (unsigned char*)malloc(o_image_data_len);
		inbuf.read((char*)o_image_data, o_image_data_len);

		if (img_ptr) free(img_ptr);

		ofstream oimg("last_finger_image.png", ios::binary);
		oimg.write((char*)o_image_data, o_image_data_len);
	}
	else throw runtime_error("Unknown image format");
}
