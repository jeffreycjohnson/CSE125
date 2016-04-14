#pragma once
#include <string>

#define METADATA_LEN 8

int encodeStruct(void * input, int inputSize, int type, char * buf, int buflen);

void decodeStruct(void * input, char * buf, int buflen, int * msgType, int * msgLen);

int decodeContentLength(std::string message);

int encodeContentLength(std::string message, char * buffer, int buflen);
