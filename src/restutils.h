#pragma once

#ifndef RESTUTILS_H
#define RESTUTILS_H

#include "AbisRest.h"

#define ELEMENT_TYPE	U("type")
#define ELEMENT_VALUE	U("value")
#define ELEMENT_ID	    U("id")
#define ELEMENT_UUID    U("uuid")
#define ELEMENT_INFO    U("info")
#define ELEMENT_COUNT	U("limit")

#define ELEMENT_RESULT	U("ok")
#define ELEMENT_ERROR	U("error")

void display_json(json::value const & jvalue, std::string const & prefix);
void handle_request(http_request request, function<void(json::value const &, json::value &)> action);

void* json2tmp(const web::json::value& el);
void* json2fingergost_tmp(const web::json::value& el);

int tmp_from_json(json::value el, int& tmp_type, void*& tmp_ptr);
int face_tmp_from_json(json::value el, int& tmp_type, void*& tmp_ptr);
int finger2_tmp_from_json(json::value el, void*& tmp_ptr, void*& gost_tmp_ptr);

#endif
