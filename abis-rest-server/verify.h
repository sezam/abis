#pragma once

#ifndef VERIFY_H
#define VERIFY_H

#include "AbisRest.h"

http_listener register_verify(uri url);
void verify_get(http_request request);

#endif
