#pragma once

#ifndef REST_COMPARE_H
#define REST_COMPARE_H

#include "AbisRest.h"

http_listener register_compare(uri url);
void compare_get(http_request request);

#endif
