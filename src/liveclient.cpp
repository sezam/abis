#include "live/Live.h"

void live_prepare()
{
#ifndef _WIN32
	//init();
#endif
}

int live_check(unsigned char* data)
{
#ifndef _WIN32
	//return Live(data);
#endif
	return 0;
}

void live_free()
{
#ifndef _WIN32
	//return clear();
#endif
}