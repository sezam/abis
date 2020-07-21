#ifndef __LIBFPRINT_H__
#define __LIBFPRINT_H__

#include "fp_internal.h"

extern "C" void getFingerPrint(struct fp_img* image, unsigned char* xyt);
extern "C" void getCurveMap(char *curve, int *wx, int *wy);
extern "C" int matchFingerPrint(unsigned char *enrollXYT, unsigned char *queredXYT);
extern "C" int matchMCC(unsigned char *enrollXYT, unsigned char *queredXYT);
extern "C" int matchSegments(struct fp_img* image1, struct fp_img* image2);
extern "C" int matchSegmentsImgTemplate(struct fp_img* image1, unsigned char* tmpl);
extern "C" int matchSegmentsTemplate(unsigned char* tmpl1, int cx1, int cy1,  unsigned char* tmpl2,int cx2,int cy2);

extern "C" int matchSegmentsTemplateArea(unsigned char* tmpl1, int cx1, int cy1,  unsigned char* tmpl2,int cx2,int cy2);
extern "C" int matchSegmentsTemplateCilindre(unsigned char* tmpl1, int cx1, int cy1,  unsigned char* tmpl2,int cx2,int cy2);

#endif // __MAIN_H__
