#pragma once
#include <string>

int decodeContentLength(std::string message);

int encodeContentLength(std::string message, char * buffer, int buflen);
