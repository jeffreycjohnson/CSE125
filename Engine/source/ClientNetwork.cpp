/*
 * PLEASE #DEFINE __LINUX IN EITHER YOUR COMPILER FLAGS
 * IF COMPILING ON NON WINDOWS MACHINES USING BSD SOCKETS
 */
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
#include "ClientNetwork.h"
#include "NetworkUtility.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#ifdef __LINUX
#else
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5000"

/**
	Default ClientNetwork Constructor

	@return an Empty ClientNetwork Object
**/
ClientNetwork::ClientNetwork() : ConnectionEstablished(false), ConnectSocket(INVALID_SOCKET)
{
}

/**
	Recommended ClientNetwork Constructor.
	Sets up the ClientNetwork object.

	@param std::string ip: string containing the ip. Preferably, in the dotted quads form, e.g. "192.168.0.1"
	@param std::string port: string containing the port. Preferably, in an integer format, e.g. "42069"
	@return an Empty ClientNetwork Object
**/
ClientNetwork::ClientNetwork(std::string ip, std::string port) {
	this->ConnectionEstablished = false;
	this->ConnectSocket = INVALID_SOCKET;
	this->serverIp = ip;
	this->port = port;
	int result = SetupTCPConnection(ip, port);
	if (result == -1) return;
}

/**
	ClientNetwork Destructor.
	Sets up the ClientNetwork object.
**/
ClientNetwork::~ClientNetwork()
{

#ifdef __LINUX
#else
	int iResult;
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
	}
	closesocket(ConnectSocket);

	WSACleanup();
#endif

	this->ConnectionEstablished = false;
	this->ConnectSocket = INVALID_SOCKET;
}

int ClientNetwork::CloseConnection(){
	int iResult;
	int closeError = 0;
	//Need to Establish Connection
	if (!ConnectionEstablished){
		std::cerr << "Already Closed" << std::endl;
		return 1;
	}

#ifdef __LINUX
#else
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
		closeError = 1;
	}
	closesocket(ConnectSocket);
#endif

	this->ConnectionEstablished = false;
	this->ConnectSocket = INVALID_SOCKET;
	return closeError;
}

int ClientNetwork::SetupTCPConnection(std::string serverIp, std::string port){

	int ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	// Initialize Winsock
#if __LINUX
#else
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}
	ZeroMemory(&hints, sizeof(hints));
#endif

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(serverIp.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
#ifdef __LINUX
#else
		WSACleanup();
#endif
		return -1;
	}

	// Attempt to connect to an address until one succeeds TODO: Handle Timeouts?
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
#ifdef __LINUX
#else
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
#endif
			return -1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
			closesocket(ConnectSocket);
#endif
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
#ifdef __LINUX
#else
		WSACleanup();
#endif
		return -1;
	}

	//Saving
	this->ConnectionEstablished = true;
	this->serverIp = serverIp;
	this->port = port;
	this->ConnectSocket = ConnectSocket;

	return 0;
}

int ClientNetwork::sendMessage(std::string message){
	int iResult;
	std::cout << "encoding content length" << std::endl;
	char buf[DEFAULT_BUFLEN];
	int contentLength = encodeContentLength(message, buf, DEFAULT_BUFLEN);

	//Need to Establish Connection
	if (!ConnectionEstablished){
		std::cerr << "Send Refused. Please Establish Connection" << std::endl;
		return 1;
	}
	std::cout << "Encoded msg: " << buf << std::endl;
	iResult = send(this->ConnectSocket, buf, contentLength, 0);
	if (iResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
		std::cerr << "Send Failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
#endif
		this->ConnectionEstablished = false;
		this->ConnectSocket = INVALID_SOCKET;
		return 1;
	}

	//Pound Define This
	printf("Bytes Sent: %d\n", iResult);

	return 0;
}

std::string ClientNetwork::receiveMessage(){
	int iResult;
	//CHANGE THE DEFAULT_BUFLEN TO DESIRE
	int buflen = DEFAULT_BUFLEN;
	char recvbuffer[DEFAULT_BUFLEN];
	memset(recvbuffer, 0, DEFAULT_BUFLEN);

	//Need to Establish Connection
	std::cout << "reached " << ConnectionEstablished << std::endl;
	if (!ConnectionEstablished){
		std::cerr << "Recv Refused. Please Establish Connection" << std::endl;
		return std::string();
	}
	//TODO: totalrecv was originally a ssize_t which is of type unsigned long, to prevent overflow..
	int totalrecv = 0;
	//struct timeval tv;
	//tv.tv_sec = 5;
	//tv.tv_usec = 0;
	//setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	std::string result = "";
	int contentLength = -1;
	do {
		if (totalrecv > buflen){
			std::cerr << "Buffer Overflow!!!!!" << std::endl;
			return std::string();
		}
		std::cout << "Receiving..." << std::endl;
		iResult = recv(ConnectSocket, recvbuffer, DEFAULT_BUFLEN-1, 0);
		std::cout << "Done!  iResult is " << iResult << std::endl;
		if (iResult > 0){
			totalrecv += iResult;
			recvbuffer[iResult] = '\0';

			if (totalrecv > sizeof(int) && result == "") { // Recieved content length of data

				contentLength = decodeContentLength(std::string(recvbuffer, sizeof(int)));
				std::cout << "Content-Length: " << contentLength << std::endl;
			} 
			result += recvbuffer;
		}
		else if (iResult == 0){
			std::cerr << "Connection Closed." << std::endl;
		}
		else{
			//Shutdown the connections
			this->ConnectionEstablished = false;
			this->ConnectSocket = INVALID_SOCKET;
			//DO we want to return a blank one or a partial one?
#ifdef __LINUX
			if (errno == EWOULDBLOCK) {
				std::cerr << "errno is " << EWOULDBLOCK << std::endl;
			}
#else
			std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
#endif
		}
		std::cout << "totalrecv: " << totalrecv << std::endl;
		if (contentLength + sizeof(int) == totalrecv) break;
	} while (iResult > 0);

	//TODO:: IS THIS ACCURATE?
	return result;
}

void ClientNetwork::GetStatus(std::string header){
	std::cout << header;
	std::cout << "Ip and Port are " << this->serverIp << ":" << this->port << std::endl;
	std::cout << "Is the connection to the port established? " << this->ConnectionEstablished << std::endl;
	std::cout << "Connect Socket is " << this->ConnectSocket << std::endl;
	std::cout << std::endl;
}



