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


#define DEFAULT_BUFLEN 512


int decodeContentLength(std::string message){
	int messageAsInt;
	if (message.size() < 4){
		return 0;
	}
	memcpy_s((void *) &messageAsInt, sizeof(int), message.c_str(), sizeof(int));
	return ntohl((u_long)messageAsInt);
}

int encodeContentLength(std::string message, char * buffer, int buflen) {
	memset(buffer, 0, buflen);

	unsigned int msgLen = message.size();
	if (msgLen > buflen - sizeof(int)) {
		printf("Error encoding Message!");
		return 0;
	}
	int len = htonl(msgLen);
	std::cout << "msgLen: " << msgLen << std::endl;
	memcpy(buffer, &len, sizeof(unsigned int));
	memcpy(buffer + sizeof(int), message.c_str(), message.size());
	std::cout << "buf: " << buffer << std::endl;
	return sizeof(int)+msgLen;

}