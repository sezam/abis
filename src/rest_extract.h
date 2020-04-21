#pragma once

#ifndef REST_EXTRACT_H
#define REST_EXTRACT_H

#include "AbisRest.h"

http_listener register_extract(uri url);
void extract_get(http_request request);

#endif
