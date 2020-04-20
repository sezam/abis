#pragma once

#ifndef ECHOTEST_H
#define ECHOTEST_H

#include "AbisRest.h"

http_listener register_echo(uri url);
void echo_get(http_request request);
void echo_post(http_request request);
void echo_put(http_request request);
void echo_del(http_request request);

#endif
