#pragma once
#include <string>
#include <vector>

#define DEFAULT_BUFLEN 2048
#define METADATA_LEN 8

struct NetworkResponse
{
	int messageType;
	std::vector<char> body;

	NetworkResponse(int messageType, std::vector<char> body) :
		messageType(messageType), body(body)
	{}

	~NetworkResponse() {}
};

struct PreviousData
{
	char buffer[DEFAULT_BUFLEN];
	int length;

	PreviousData()
	{
		memset(buffer, '\0', sizeof(char) * DEFAULT_BUFLEN);
		length = 0;
	}

	~PreviousData() {}
};

std::vector<char> encodeMessage(std::vector<char> message, int messageType);

int encodeStruct(void * input, int inputSize, int type, char * buf, int buflen);

std::vector<char> decodeStruct(char * buf, int buflen, int * msgType, int * msgLen);

int decodeContentLength(std::string message);

int encodeContentLength(std::string message, char * buffer, int buflen);
