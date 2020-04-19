#pragma once

#ifndef EXTRACT_H
#define EXTRACT_H

#include "AbisRest.h"

http_listener register_extract(uri url);
void extract_get(http_request request);

#endif
