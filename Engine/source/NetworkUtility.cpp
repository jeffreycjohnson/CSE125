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

// Returns int representing type of message
void decodeStruct(void * input, char * buf, int buflen, int * msgType, int * msgLen) {
	int inputSize = 0;

	if (buflen < METADATA_LEN){ // Buffer can't store contentLength and msgType
		if (DEBUG) std::cerr << "Error decoding struct!" << std::endl;
		return;
	}

	memcpy(msgLen, buf, sizeof(int));
	*msgLen = ntohl(*msgLen);
	memcpy(msgType, buf + sizeof(int), sizeof(int));
	*msgType = ntohl(*msgType);

	if (*msgType == INPUT_NETWORK_DATA)
	{
		inputSize = sizeof(InputNetworkData);
	}

	memcpy(input, buf + METADATA_LEN, inputSize);
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