#pragma once

#ifndef LIVECLIENT_H
#define LIVECLIENT_H

void live_prepare();
int live_check(unsigned char* data);
void live_free();

#endif