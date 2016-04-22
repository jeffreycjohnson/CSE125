#pragma once
#include <string>
#include <vector>

#define DEFAULT_BUFLEN 2048
#define METADATA_LEN 12
#define METADATA_CNT 3

struct NetworkResponse
{
	int messageType;
	int id;
	std::vector<char> body;

	NetworkResponse(int messageType, int msgId, std::vector<char> body) :
		messageType(messageType), id(msgId), body(body)
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

std::vector<char> encodeMessage(std::vector<char> message, int messageType, int id);

std::vector<char> decodeMessage(char * buf, int buflen, int * msgType, int * id, int * msgLen);

int decodeContentLength(std::string message);

int encodeContentLength(std::string message, char * buffer, int buflen);
