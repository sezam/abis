#pragma once

#ifndef BIOCARD_H
#define BIOCARD_H

#include "AbisRest.h"

http_listener register_biocard(uri url);
void biocard_get(http_request request);
void biocard_post(http_request request);
void biocard_put(http_request request);
void biocard_patch(http_request request);
//void biocard_delete(http_request request);

#endif
