#pragma once

#ifndef EBSCLIENT_H
#define EBSCLIENT_H

#define FACESIZE 512

void saveToLog(int elev, char *log, char *data);

int get_face_template(const unsigned char *data, unsigned int dataLen, float *fTemplate);

void ebs_client_init();

void ebs_client_done();

#endif
