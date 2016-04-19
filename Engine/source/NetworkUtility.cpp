#include "NetworkUtility.h"
#ifdef __LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif
#include <iostream>
#include "NetworkStruct.h"

#define DEFAULT_BUFLEN 512
#define DEBUG 0

std::vector<char> encodeMessage(std::vector<char> message, int messageType)
{
	std::vector<char> encodedMsg;
	encodedMsg.resize(METADATA_LEN + message.size());

	// get extra params
	int rawByteLen = htonl(METADATA_LEN + message.size());
	int encodedType = htonl(messageType);

	//
	memcpy(encodedMsg.data(), &rawByteLen, sizeof(int));
	memcpy(encodedMsg.data() + sizeof(int), &encodedType, sizeof(int));
	memcpy(encodedMsg.data() + METADATA_LEN, message.data(), message.size());

	return encodedMsg;
}

int encodeStruct(void * input, int inputSize, int type, char * buf, int buflen) {
	memset(buf, 0, buflen);
	int rawByteLen = htonl(inputSize + METADATA_LEN);
	if (inputSize + METADATA_LEN > buflen) {
		if (DEBUG) std::cerr << "Error encoding struct!" << std::endl;
		return 0;
	}	
	
	int encodedType = htonl(type);
	memcpy(buf, &rawByteLen, sizeof(int));
	memcpy(buf + sizeof(int), &encodedType, sizeof(int));
	memcpy(buf + METADATA_LEN, input, inputSize);
	return inputSize + METADATA_LEN;
}

std::vector<char> decodeStruct(char * buf, int buflen, int * msgType, int * msgLen) {
	int inputSize = 0;

	std::vector<char> empty;

	if (buflen < METADATA_LEN){ // Buffer can't store contentLength and msgType
		if (DEBUG) std::cerr << "Error decoding struct!" << std::endl;
		return empty;
	}
	
	memcpy(msgLen, buf, sizeof(int));
	*msgLen = ntohl(*msgLen);
	memcpy(msgType, buf + sizeof(int), sizeof(int));
	*msgType = ntohl(*msgType);

	inputSize = NetworkStruct::sizeOf(*msgType);
	std::vector<char> input(buf + METADATA_LEN, buf + METADATA_LEN + inputSize);

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