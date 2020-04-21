#pragma once

#ifndef REST_VERIFY_H
#define REST_VERIFY_H

#include "AbisRest.h"

http_listener register_verify(uri url);
void verify_get(http_request request);

#endif
