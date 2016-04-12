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

#include "NetworkUtility.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "ServerNetwork.h"
#include <iostream>

// Need to link with Ws2_32.lib
#ifdef __LINUX
#else
#pragma comment (lib, "Ws2_32.lib")
#endif
// #pragma comment (lib, "Mswsock.lib")

ServerNetwork::ServerNetwork(std::string port) {
	this->port = port;
	std::cout << "Starting server on port " << port << std::endl;
	listenSocket = setupSocket(port);
	
	if (listenSocket == -1) {
		printf("Error in setupSocket()");
	}
}


ServerNetwork::~ServerNetwork() {
	// shutdown the connection since we're done
	std::cout << "calling destructor" << std::endl;
#ifdef __LINUX
#else
	int iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(clientSocket);
	WSACleanup();
#endif
}

void ServerNetwork::start() {
	clientSocket = acceptTCPConnection(listenSocket);

	while (true) {
		handleClient(clientSocket);
		//break;
	}
}

void ServerNetwork::handleClient(int clientSocket) {
		// Receive until the peer shuts down the connection
	int iResult;
	std::string data = "";
	int totalBytesRecvd = 0;
	int contentLength = -1;
	do {
		char recvbuf[DEFAULT_BUFLEN];
		iResult = recv(clientSocket, recvbuf, DEFAULT_BUFLEN-1, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			totalBytesRecvd += iResult;
			recvbuf[iResult] = '\0';
			if (totalBytesRecvd > sizeof(int) && data == "") {
				contentLength = decodeContentLength(std::string(recvbuf, sizeof(int)));
				std::cout << "Content-Length: " << contentLength << std::endl;

			}
			data += recvbuf + 4;
			std::cout << "Received the following data: " << data << std::endl;
			char encodedMsg[DEFAULT_BUFLEN];

			int contentLength = encodeContentLength(data, encodedMsg, DEFAULT_BUFLEN);
			// Echo the buffer back to the sender
			int iSendResult = send(clientSocket, encodedMsg, contentLength, 0);
			if (iSendResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
#endif
				return;
			}
			printf("Bytes sent: %d\n", iSendResult);
			#ifdef __LINUX
				close(clientSocket);
			#endif
			return;
		}
		else if (iResult == 0) {
			//printf(".");
			break;
		}
		else  {
#ifdef __LINUX
#else
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
#endif
			return;
		}
		if (contentLength == totalBytesRecvd) break;

	} while (iResult > 0);
}

int ServerNetwork::acceptTCPConnection(int listenSocket) {
	// Accept a client socket
	int clientSocket;
	clientSocket = accept(listenSocket, NULL, NULL);

	if (clientSocket == INVALID_SOCKET) {
#ifdef __LINUX
#else
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
#endif
		return -1;
	}

	// No longer need server socket
#ifdef __LINUX
#else
	closesocket(listenSocket);
#endif

	return clientSocket;
}

int ServerNetwork::setupSocket(std::string port) {

#ifdef __LINUX
#else
	WSADATA wsaData;
#endif

	int iResult;

	int ListenSocket = INVALID_SOCKET;
	int ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
#ifdef __LINUX
#else
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}
#ifdef __LINUX
#else
	ZeroMemory(&hints, sizeof(hints));
#endif

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
#ifdef __LINUX
#else
		WSACleanup();
#endif
		return -1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
#ifdef __LINUX
#else
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
#endif
		return -1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
#endif
		return -1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
#endif
		return -1;
	}
	return ListenSocket;
}
