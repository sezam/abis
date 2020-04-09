#pragma once

#ifndef RESTUTILS_H
#define RESTUTILS_H

#include "AbisRest.h"

void display_json(json::value const & jvalue, utility::string_t const & prefix);
void handle_request(http_request request, function<void(json::value const &, json::value &)> action);

#endif
