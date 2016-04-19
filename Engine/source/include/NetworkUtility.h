#pragma once
#include <string>
#include <vector>

#define METADATA_LEN 8

std::vector<char> encodeMessage(std::vector<char> message, int messageType);

int encodeStruct(void * input, int inputSize, int type, char * buf, int buflen);

std::vector<char> decodeStruct(char * buf, int buflen, int * msgType, int * msgLen);

int decodeContentLength(std::string message);

int encodeContentLength(std::string message, char * buffer, int buflen);
