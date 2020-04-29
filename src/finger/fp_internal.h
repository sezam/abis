#ifndef __FPRINT_INTERNAL_H__
#define __FPRINT_INTERNAL_H__

#define array_n_elements(array) (sizeof(array) / sizeof(array[0]))

#define FP_IMGDRV_SUPPORTS_UNCONDITIONAL_CAPTURE (1 << 0)
#define gboolean unsigned int

struct fp_minutiae {
	int alloc;
	int num;
	struct fp_minutia** list;
};

/* bit values for fp_img.flags */
#define FP_IMG_V_FLIPPED		(1<<0)
#define FP_IMG_H_FLIPPED		(1<<1)
#define FP_IMG_COLORS_INVERTED	(1<<2)
#define FP_IMG_BINARIZED_FORM	(1<<3)
#define FP_IMG_PARTIAL			(1<<4)

#define FP_IMG_STANDARDIZATION_FLAGS (FP_IMG_V_FLIPPED | FP_IMG_H_FLIPPED | FP_IMG_COLORS_INVERTED)

struct fp_img {
	int width;
	int height;
	size_t length;
	uint16_t flags;
	struct fp_minutiae* minutiae;
	unsigned char* binarized;
	unsigned char data[0];
};

int img_to_print_data(struct fp_img* img, unsigned char* xyt);
void getCurve(char* map, int* wx, int* wy);

#endif

