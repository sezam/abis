#pragma once

#ifndef COMPARE_H
#define COMPARE_H

#include "AbisRest.h"

http_listener register_compare(uri url);
void compare_get(http_request request);

#endif
