#include "live/Live.h"

void live_prepare()
{
#ifndef _WIN32
	init(threshold_min, threshold_max, ir_threshold, fish_threshold, ir_threshold_select, ensembled_threshold);
#endif
}

int live_check(unsigned char* data)
{
#ifndef _WIN32
	return Live(data);
#endif
	return 0;
}

void live_free()
{
#ifndef _WIN32
	return clear();
#endif
}