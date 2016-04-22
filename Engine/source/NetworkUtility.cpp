#include "NetworkUtility.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include "NetworkStruct.h"

#define DEFAULT_BUFLEN 2048
#define DEBUG 0

std::vector<char> encodeMessage(const std::vector<char> &message, int messageType, int id)
{
	std::vector<char> encodedMsg;
	encodedMsg.resize(METADATA_LEN + message.size() * sizeof(char));

	// get extra params
	int rawByteLen = htonl(METADATA_LEN + message.size() * sizeof(char));
	int encodedType = htonl(messageType);
	int encodedId = htonl(id);
	
	*((int*)(encodedMsg.data())) = rawByteLen;
	*((int*)(encodedMsg.data() + sizeof(int))) = encodedType;
	*((int*)(encodedMsg.data() + sizeof(int) + sizeof(int))) = encodedId;
	memcpy(encodedMsg.data() + METADATA_LEN, message.data(), message.size());

	return encodedMsg;
}

std::vector<char> decodeMessage(char * buf, int buflen, int * msgType, int * id, int * msgLen) {
	int inputSize = 0;

	std::vector<char> empty;

	if (buflen < METADATA_LEN){ // Buffer can't store contentLength and msgType
		if (DEBUG) std::cerr << "Error decoding struct!" << std::endl;
		return empty;
	}
	
	*msgLen = ntohl(*((int*)(buf)));
	*msgType = ntohl(*((int*)(buf + sizeof(int))));
	*id = ntohl(*((int*)(buf + sizeof(int) + sizeof(int))));

	if (*msgType == 0)
	{
		std::vector<char> input(buf, buf + *msgLen);
		buflen += 3;
		buflen -= 3;
	}

	inputSize = NetworkStruct::sizeOf(*msgType);
	std::vector<char> input(buf + METADATA_LEN, buf + *msgLen);

	return input;
}

int decodeContentLength(std::string message){
	int messageAsInt;
	if (message.size() < 4){
		if (DEBUG) std::cerr << "Error decoding Content Length!" << std::endl;
		return 0;
	}
	memcpy_s((void *) &messageAsInt, sizeof(int), message.c_str(), sizeof(int));
	return ntohl((u_long)messageAsInt);
}

int encodeContentLength(std::string message, char * buffer, int buflen) {
	memset(buffer, 0, buflen);

	unsigned int msgLen = message.size();
	if (msgLen > buflen - sizeof(int)) {
		if (DEBUG) std::cerr << "Error encoding Content Length!" << std::endl;
		return 0;
	}
	int len = htonl(msgLen);
	memcpy(buffer, &len, sizeof(unsigned int));
	memcpy(buffer + sizeof(int), message.c_str(), message.size());
	return sizeof(int)+msgLen;

}