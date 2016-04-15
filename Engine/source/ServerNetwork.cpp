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
#include "NetworkStruct.h"
#include "ServerNetwork.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <iostream>
#include <vector>

#include <stdexcept>

// Need to link with Ws2_32.lib
#ifdef __LINUX
#else
#pragma comment (lib, "Ws2_32.lib")
#endif
// #pragma comment (lib, "Mswsock.lib")

int ServerNetwork::clientSocket;
int ServerNetwork::listenSocket;
std::string ServerNetwork::port;

char ServerNetwork::lastBuf[DEFAULT_BUFLEN];
int ServerNetwork::lastLen = 0;

void ServerNetwork::setup(std::string port)
{
	ServerNetwork::port = port;
	std::cout << "Starting server on port " << port << std::endl;
	ServerNetwork::listenSocket = setupSocket(port);

	if (listenSocket == -1) {
		printf("Error in setupSocket()");

#ifdef _EXCEPTIONAL
		throw new std::runtime_error("Error in setupSocket()");
#endif
	}
}

void ServerNetwork::closeConnection()
{
	// shutdown the connection since we're done
	std::cout << "calling destructor" << std::endl;
#ifdef __LINUX
#else
	int iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);

# ifdef _EXCEPTIONAL
		std::string message = "shutdown failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif
		WSACleanup();

		return;
	}

	// cleanup
	closesocket(clientSocket);
	WSACleanup();
#endif
}

void ServerNetwork::start() {
	ServerNetwork::clientSocket = acceptTCPConnection(ServerNetwork::listenSocket);
	std::cout << "Accepted tcp connection" << std::endl;
}

std::vector<NetworkResponse> ServerNetwork::handleClient() { return handleClient(clientSocket); }

std::vector<NetworkResponse>ServerNetwork::handleClient(int clientSocket) {
	// Receive until the peer shuts down the connection
	int iResult;
	int totalBytesRecvd = 0;
	int contentLength = -1;

	std::vector<NetworkResponse> msgs;
	std::vector<char> msg;

	// non-blocking mode is enabled.
	u_long iMode = 1;
	ioctlsocket(clientSocket, FIONBIO, &iMode);

	char recvbuf[DEFAULT_BUFLEN];

	if (ServerNetwork::lastLen != 0)
	{
		memcpy(recvbuf, ServerNetwork::lastBuf, DEFAULT_BUFLEN);
		totalBytesRecvd += ServerNetwork::lastLen;
	}

	do {
		iResult = recv(ServerNetwork::clientSocket, recvbuf + totalBytesRecvd, DEFAULT_BUFLEN - totalBytesRecvd - 1 , 0);

		int nError = WSAGetLastError();
		if (nError == WSAEWOULDBLOCK) {
			break;
		}

		if (iResult > 0) {
			totalBytesRecvd += iResult;
			int msgLength = 0;
			int totalBytesProcd = 0;

			// only read a message once we've gotten enough bytes tbh
			do
			{
				// what do if we don't even have enough for the message length
				if ((totalBytesRecvd - totalBytesProcd) < sizeof(int))
				{
					memcpy(ServerNetwork::lastBuf, recvbuf + totalBytesProcd, DEFAULT_BUFLEN - totalBytesProcd);
					ServerNetwork::lastLen = totalBytesRecvd - totalBytesProcd;

					break;
				}

				// grab the message length to see if we have enough
				memcpy(&msgLength, recvbuf + totalBytesProcd, sizeof(int));
				msgLength = ntohl(msgLength);

				// what do if we don't have enough bytes for the whole message
				if ((totalBytesRecvd - totalBytesProcd) < msgLength)
				{
					memcpy(ServerNetwork::lastBuf, recvbuf + totalBytesProcd, DEFAULT_BUFLEN - totalBytesProcd);
					ServerNetwork::lastLen = totalBytesRecvd - totalBytesProcd;

					break;
				}

				// parse the whole message
				int msgType = -1;
				msg = decodeStruct(recvbuf + totalBytesProcd, DEFAULT_BUFLEN, &msgType, &contentLength);
				totalBytesProcd += contentLength;

				NetworkResponse response(msgType, msg);
				msgs.push_back(response);
			}
			while (totalBytesRecvd > totalBytesProcd);
			
			std::cerr << "We found " << msgs.size() << " messages" << std::endl;
			break;

		}
		else if (iResult == 0) {
			break;
		}
		else  {
#ifdef __LINUX
#else
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);

# ifdef _EXCEPTIONAL
			std::string message = "recv failed with error: ";
			message += WSAGetLastError();
			throw new std::runtime_error(message);
# endif
			WSACleanup();
#endif
			return msgs;
		}

	} while (iResult > 0);

	return msgs;
}

int ServerNetwork::sendMessage(void * message, int msgType) {
	char encodedMsg[DEFAULT_BUFLEN];
	int contentLength = encodeStruct(message, NetworkStruct::sizeOf(msgType), msgType, encodedMsg, DEFAULT_BUFLEN);

	int iSendResult = send(ServerNetwork::clientSocket, encodedMsg, contentLength, 0);
	if (iSendResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
		int wsaLastError = WSAGetLastError();
		if (wsaLastError != WSAEWOULDBLOCK)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ServerNetwork::clientSocket);
# ifdef _EXCEPTIONAL
			std::string message = "send failed with error: ";
			message += WSAGetLastError();
			throw new std::runtime_error(message);
# endif

			WSACleanup();
		}
#endif
		return 1;
	}
#ifdef __LINUX
	close(clientSocket);
#endif
	return 0;
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
# ifdef _EXCEPTIONAL
		std::string message = "accept failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

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
# ifdef _EXCEPTIONAL
		std::string message = "WSAStartup failed with error: ";
		message += iResult;
		throw new std::runtime_error(message);
# endif
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
# ifdef _EXCEPTIONAL
		std::string message = "getaddrinfo failed with error: ";
		message += iResult;
		throw new std::runtime_error(message);
# endif

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
# ifdef _EXCEPTIONAL
		std::string message = "socket failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

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
# ifdef _EXCEPTIONAL
		std::string message = "bind failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

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
# ifdef _EXCEPTIONAL
		std::string message = "listen failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

		WSACleanup();
#endif
		return -1;
	}
	return ListenSocket;
}
