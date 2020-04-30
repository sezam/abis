#pragma once

#ifndef REST_BIOCARD_H
#define REST_BIOCARD_H

#include "AbisRest.h"

http_listener register_biocard(uri url);
void biocard_get(http_request request);
void biocard_del(http_request request);
void biocard_put(http_request request);
#endif
