#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "bmp.h"

#define Sigma 10.0
#define Sigma_2D 5.0
// Cosin component
#define Lambda 20
#define Lambda_2D 20
// Aspect ratio
#define Gamma 0.5
// Direction
#define Direction 8
// Coordinate width for gabor
#define K1 3
#define K2 3
#define K1_1D 25
#define K1_2D 25
#define K2_2D 25


void tBitmap::imgMean(int size)
{
	unsigned int x, y, a, c;
	int p, q, s;
	unsigned char* tmp;
	tmp = new unsigned char[wid * hei];
	for (unsigned int i = 0;i < wid * hei;i++) *(tmp + i) = *(pimage + i);

	s = size / 2;
	a = size * size;

	for (y = s; y < hei - s; y++)
	{
		for (x = s; x < wid - s; x++)
		{
			c = 0;
			for (q = -s; q <= s; q++)
			{
				for (p = -s; p <= s; p++)
				{
					c += *(tmp + (y + q) * wid + (x + p));
				}
			}
			*(pimage + (y * wid) + x) = c / a;
		}
	}
	delete[] tmp;
}

tBitmap::tBitmap()
{
	left = 0;
	top = 0;
	wid = 0;
	hei = 0;
	al = 0xFF;
	vis = false;
	pimage = NULL;
	colorKey = 0;
	colorKeyPresent = false;
	image = NULL;
	grayImage = NULL;
}

void tBitmap::setPosition(int x, int y)
{
	left = x;
	top = y;
}

int tBitmap::loadBitmap(unsigned char* image_data, const unsigned int image_len)
{
	bmpheader* header = (bmpheader*)image_data;
	if (image != NULL) delete this->image;

	image = new etna_bo;
	if (image == NULL) return -1;

	image->width = header->Width;
	image->height = header->Height;
	image->image = new unsigned int[header->Width * header->Height];
	if (image->image == NULL) return -2;

	pimage = (uint32_t*)image->image;

	unsigned char* buf = &image_data[header->DataOffset];
	unsigned char ch, r, g, b;
	unsigned int col, ddis;
	int nc;

	switch (header->BitsPerPixel)
	{
	case 1:
		x = 0;
		y = 0;
		for (int n = 0;n < header->DataSize; n++)
		{
			ch = buf[n];
			do
			{
				nc = ch & 1;
				ch = ch >> 1;
				r = header->pal[nc].R;
				g = header->pal[nc].G;
				b = header->pal[nc].B;
				col = RGBToColor(0, r, g, b);
				ddis = ((header->Height - 1 - y) * header->Width + x);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
				x++;
				if (x == header->Width)
				{
					x = 0;
					y++;
				}
			} while (ch > 0);
		}
		break;
	case 4:
		res = fread(buf, head.DataSize, 1, bmpFile);
		x = 0;
		y = 0;
		for (n = 0;n < head.DataSize;n++)
		{
			nc = buf[n] / 16;
			nc1 = buf[n] % 16;
			r = head.pal[nc].R;
			g = head.pal[nc].G;
			b = head.pal[nc].B;
			col = RGBToColor(0, r, g, b);
			ddis = dis + ((heigth - 1 - y) * width + x);
			if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			x++;
			if (x == head.Width)
			{
				x = 0;
				y++;
			}
			r = head.pal[nc1].R;
			g = head.pal[nc1].G;
			b = head.pal[nc1].B;
			col = RGBToColor(0, r, g, b);
			ddis = dis + ((heigth - 1 - y) * width + x);
			if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			x++;
			if (x == head.Width)
			{
				x = 0;
				y++;
			}
		}
		break;
	case 8:
		if (head.Width % 4 == 0) strlen = head.Width; else strlen = head.Width + (4 - head.Width % 4);
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 1, strlen, bmpFile);
			for (x = 0;x < head.Width;x++)
			{
				r = head.pal[buf[x]].R;
				g = head.pal[buf[x]].G;
				b = head.pal[buf[x]].B;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
		break;
	case 16:
		if ((head.Width * 2) % 4 == 0) strlen = head.Width * 2; else strlen = head.Width * 2 + 2;
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 2, strlen, bmpFile);
			n = 0;
			for (x = 0;x < head.Width;x++)
			{
				col = buf[n + 1] * 256 + buf[n];
				n = n + 2;
				b = (col % 32) * 8;
				col = col / 32;
				g = (col % 32) * 8;
				col = col / 32;
				r = (col % 32) * 8;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
		break;
	case 24:
		if ((head.Width * 3) % 4 == 0) strlen = head.Width * 3; else strlen = head.Width * 3 + (4 - (head.Width * 3) % 4);
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 3, head.Width, bmpFile);
			n = 0;
			for (x = 0;x < head.Width;x++)
			{
				b = buf[n];
				g = buf[n + 1];
				r = buf[n + 2];
				n = n + 3;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
		break;
	case 32:
	{
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 4, head.Width, bmpFile);
			n = 0;
			for (x = 0;x < head.Width;x++)
			{
				b = buf[n];
				g = buf[n + 1];
				r = buf[n + 2];
				n = n + 4;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
	}
	break;
	default:
		break;
	};

}


void tBitmap::loadBitmap(char* name)
{
	bmpheader head;
	int nc1, nc, n, width, heigth, col;
	int x, y;
	unsigned char* buf;
	buf = (unsigned char*)malloc(32768);
	int ddis, r, g, b;
	unsigned char ch;
	FILE* bmpFile = NULL;
	bmpFile = fopen(name, "rb");
	vis = false;
	if (bmpFile == NULL)
	{
		printf("Error, File %s not found.\n", name);
		exit(1);
	}
	int res;
	res = fread(&head, sizeof(head), 1, bmpFile);
	if (res != 1)
	{
		printf("Error reading file %s %d\n", name, res);
	}
	fseek(bmpFile, head.DataOffset, SEEK_SET);
	width = head.Width;
	heigth = head.Height;
	wid = width;
	hei = heigth;
	if (image != NULL) delete image;
	image = new etna_bo;
	if (image == NULL)
	{
		printf("do not allocated memory. Aborted.\n");
		exit(1);
	}
	image->width = wid;
	image->height = hei;
	image->image = new unsigned int[wid * hei];
	if (image->image == NULL)
	{
		printf("do not allocated memory. Aborted.\n");
		exit(1);
	}
	int dis = 0;
	void* pimage1 = image->image;
	pimage = (uint32_t*)pimage1;
	int strlen;
	switch (head.BitsPerPixel)
	{
	case 1:
		res = fread(buf, head.DataSize, 1, bmpFile);
		x = 0;
		y = 0;
		for (n = 0;n < head.DataSize;n++)
		{
			ch = buf[n];
			do
			{
				nc = ch & 1;
				ch = ch >> 1;
				r = head.pal[nc].R;
				g = head.pal[nc].G;
				b = head.pal[nc].B;
				col = RGBToColor(0, r, g, b);
				ddis = dis + ((heigth - 1 - y) * width + x);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
				x++;
				if (x == head.Width)
				{
					x = 0;
					y++;
				}
			} while (ch > 0);
		}
		break;
	case 4:
		res = fread(buf, head.DataSize, 1, bmpFile);
		x = 0;
		y = 0;
		for (n = 0;n < head.DataSize;n++)
		{
			nc = buf[n] / 16;
			nc1 = buf[n] % 16;
			r = head.pal[nc].R;
			g = head.pal[nc].G;
			b = head.pal[nc].B;
			col = RGBToColor(0, r, g, b);
			ddis = dis + ((heigth - 1 - y) * width + x);
			if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			x++;
			if (x == head.Width)
			{
				x = 0;
				y++;
			}
			r = head.pal[nc1].R;
			g = head.pal[nc1].G;
			b = head.pal[nc1].B;
			col = RGBToColor(0, r, g, b);
			ddis = dis + ((heigth - 1 - y) * width + x);
			if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			x++;
			if (x == head.Width)
			{
				x = 0;
				y++;
			}
		}
		break;
	case 8:
		if (head.Width % 4 == 0) strlen = head.Width; else strlen = head.Width + (4 - head.Width % 4);
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 1, strlen, bmpFile);
			for (x = 0;x < head.Width;x++)
			{
				r = head.pal[buf[x]].R;
				g = head.pal[buf[x]].G;
				b = head.pal[buf[x]].B;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
		break;
	case 16:
		if ((head.Width * 2) % 4 == 0) strlen = head.Width * 2; else strlen = head.Width * 2 + 2;
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 2, strlen, bmpFile);
			n = 0;
			for (x = 0;x < head.Width;x++)
			{
				col = buf[n + 1] * 256 + buf[n];
				n = n + 2;
				b = (col % 32) * 8;
				col = col / 32;
				g = (col % 32) * 8;
				col = col / 32;
				r = (col % 32) * 8;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
		break;
	case 24:
		if ((head.Width * 3) % 4 == 0) strlen = head.Width * 3; else strlen = head.Width * 3 + (4 - (head.Width * 3) % 4);
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 3, head.Width, bmpFile);
			n = 0;
			for (x = 0;x < head.Width;x++)
			{
				b = buf[n];
				g = buf[n + 1];
				r = buf[n + 2];
				n = n + 3;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
		break;
	case 32:
	{
		for (y = 0;y < head.Height;y++)
		{
			res = fread(buf, 4, head.Width, bmpFile);
			n = 0;
			for (x = 0;x < head.Width;x++)
			{
				b = buf[n];
				g = buf[n + 1];
				r = buf[n + 2];
				n = n + 4;
				ddis = dis + ((heigth - 1 - y) * width + x);
				col = RGBToColor(0, r, g, b);
				if (colorKeyPresent) *(pimage + ddis) = col; else *(pimage + ddis) = col | (al << 24);
			}
		}
	}
	break;
	default:
		break;
	};
	free(buf);
	fclose(bmpFile);
}


void tBitmap::getData(unsigned int* width, unsigned int* height, unsigned int data[])
{
	if (pimage != NULL)
	{
		for (int j = 0;j<int(wid * hei);j++)
		{
			data[j] = *(pimage + j);
		}
	}
}

void tBitmap::visible(bool f)
{
	vis = f;
}

int tBitmap::getVisible()
{
	return vis;
}

void tBitmap::setalpha(unsigned char a)
{
	if (a != al)
	{
		al = a;
		if (pimage != NULL)
		{
			if (colorKeyPresent)
			{
				for (unsigned int i = 0;i < wid * hei;i++)
				{
					unsigned int cl = *(pimage + i) & 0x00FFFFFF;
					if (cl == colorKey)
					{
						*(pimage + i) = cl;
					}
					else
					{
						*(pimage + i) = cl | (al << 24);
					}
				}
			}
			else
			{
				for (unsigned int i = 0;i < wid * hei;i++)
				{
					*(pimage + i) = ((*(pimage + i) & 0x00FFFFFF) | (al << 24));
				}
			}
		}
	}
}

void tBitmap::setColorKey(unsigned int col)
{
	colorKeyPresent = true;
	colorKey = col & 0x00FFFFFF;
	if (pimage != NULL)
	{
		for (unsigned int i = 0;i < wid * hei;i++)
		{
			unsigned int cl = *(pimage + i) & 0x00FFFFFF;
			if (cl == colorKey)
			{
				*(pimage + i) = cl;
			}
		}
	}
}

int tBitmap::getWidth()
{
	return wid;
}

int tBitmap::getHeight()
{
	return hei;
}

unsigned int* tBitmap::getImagePointer()
{
	return pimage;
}

void tBitmap::clearVisible()
{
	vis = false;
}

void tBitmap::refrashImage()
{
	//if (vis&&image!=NULL) addBitmapToQueue(image,left,top,wid,hei);
}

void tBitmap::destroyImage()
{
	if (grayImage != NULL) delete[] grayImage;
	grayImage = NULL;
	if (image != NULL)
	{
		if (image->image != NULL) delete[] image->image;
		image->image = NULL;
		image->width = 0;
		image->height = 0;
		delete image;
	}
	image = NULL;
	pimage = NULL;
	vis = false;
	colorKey = 0;
	colorKeyPresent = false;
	al = 0;
}

unsigned int tBitmap::RGBToColor(unsigned char alpha, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned int color = 0;
	color = ((unsigned int)alpha << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | ((unsigned int)b);
	return color;
}

void tBitmap::colorToRGB(unsigned int color, unsigned char* alpha, unsigned char* r, unsigned char* g, unsigned char* b)
{
	*alpha = (color & 0xFF000000) >> 24;
	*r = (color & 0x00FF0000) >> 16;
	*g = (color & 0x0000FF00) >> 8;
	*b = (color & 0x000000FF);
}

void tBitmap::makeGrayImage()
{
	if (grayImage) delete[] grayImage;
	grayImage = new unsigned char[wid * hei];
	if (grayImage)
	{
		for (unsigned int i = 0; i < hei * wid;i++)
		{
			unsigned int cl = *(pimage + i);
			unsigned char a, r, g, b;
			colorToRGB(cl, &a, &r, &g, &b);
			float cl8 = (float)r * (float)0.3 + (float)g * (float)0.59 + (float)b * (float)0.11;
			*(grayImage + i) = (unsigned char)cl8;
		}
	}
}

unsigned char* tBitmap::getGrayImagePointer()
{
	return grayImage;
}

