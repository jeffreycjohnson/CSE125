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
	int forClient;

	NetworkResponse(){}
	NetworkResponse(int messageType, int msgId, const std::vector<char>& body, int forClient = -1) :
		messageType(messageType), id(msgId), body(body), forClient(forClient)
	{
	}

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

std::vector<char> encodeMessage(const std::vector<char> &message, int messageType, int id);
std::vector<char> decodeMessage(char * buf, int buflen, int * msgType, int * id, int * msgLen);

int decodeContentLength(std::string message);
int encodeContentLength(std::string message, char * buffer, int buflen);

template<class T>
std::vector<char> structToBytes(T st)
{
	std::vector<char> bytes;
	bytes.resize(sizeof(st));
	memcpy(bytes.data(), &st, sizeof(st));

	return bytes;
}

template<class T>
T structFromBytes(std::vector<char> bytes)
{
	return *((T*)bytes.data());
}