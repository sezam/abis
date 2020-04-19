#pragma once

#ifndef SEARCH_H
#define SEARCH_H

#include "AbisRest.h"

http_listener register_search(uri url);
void search_get(http_request request);

#endif
