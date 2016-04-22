#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "NetworkStruct.h"
#include "ServerNetwork.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <iostream>
#include <vector>

#include <stdexcept>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

int ServerNetwork::listenSocket;
std::string ServerNetwork::port;

std::vector<int> ServerNetwork::clients;
std::unordered_map<int, PreviousData> ServerNetwork::previousClientData;

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

	for (int clientSocket : clients)
	{
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
	}
}

std::vector<NetworkResponse>ServerNetwork::handleClient(int clientSocket) 
{
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
	PreviousData& prevData = ServerNetwork::previousClientData[clientSocket];

	if (prevData.length != 0)
	{
		memcpy(recvbuf, prevData.buffer, DEFAULT_BUFLEN);
		totalBytesRecvd += prevData.length;
	}

	do {
		iResult = recv(clientSocket, recvbuf + totalBytesRecvd, DEFAULT_BUFLEN - totalBytesRecvd - 1 , 0);

		int nError = WSAGetLastError();
		if (nError == WSAEWOULDBLOCK) {
			break;
		}

		if (iResult > 0) {
			totalBytesRecvd += iResult;
			int msgLength = 0;
			unsigned int totalBytesProcd = 0;

		// only read a message once we've gotten enough bytes tbh
		do
		{
			// what do if we don't even have enough for the message length
			if ((totalBytesRecvd - totalBytesProcd) < sizeof(int))
			{
				memset(prevData.buffer, 0, DEFAULT_BUFLEN);
				memcpy(prevData.buffer, recvbuf + totalBytesProcd, DEFAULT_BUFLEN - totalBytesProcd);
				prevData.length = totalBytesRecvd - totalBytesProcd;

				break;
			}

			// grab the message length to see if we have enough
			msgLength = ntohl(*((int *)(recvbuf + totalBytesProcd)));

			// what do if we don't have enough bytes for the whole message
			if ((totalBytesRecvd - totalBytesProcd) < msgLength)
			{
				memset(prevData.buffer, 0, DEFAULT_BUFLEN);
				memcpy(prevData.buffer, recvbuf + totalBytesProcd, DEFAULT_BUFLEN - totalBytesProcd);
				prevData.length = totalBytesRecvd - totalBytesProcd;

				break;
			}

			// parse the whole message
			int msgType = -1;
			int msgId = -1;
			msg = decodeMessage(recvbuf + totalBytesProcd, DEFAULT_BUFLEN, &msgType, &msgId, &contentLength);
			totalBytesProcd += contentLength;

			NetworkResponse response(msgType, msgId, msg);
			msgs.push_back(response);
		}
		while (totalBytesRecvd > totalBytesProcd);
			
			//std::cerr << clientSocket << ": We found " << msgs.size() << " messages" << std::endl;
			break;
		}
		else if (iResult == 0) 
		{
			break;
		}
		else  
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);

# ifdef _EXCEPTIONAL
			std::string message = "recv failed with error: ";
			message += WSAGetLastError();
			throw new std::runtime_error(message);
# endif
			WSACleanup();
			return msgs;
		}

	} while (iResult > 0);

	return msgs;
}

std::vector<int> ServerNetwork::startMultiple(int numClients)
{
	std::vector<int> clientIDs;

	for (int sofar = 0; sofar < numClients; sofar++)
	{
		// we don't need to crash and burn here, let's just crash and burn in acceptTCPConnection
		int potentialClient = acceptTCPConnection(ServerNetwork::listenSocket);

		clientIDs.push_back(sofar);

		// add the clientID -> socket mapping
		ServerNetwork::clients.push_back(potentialClient);

		// and create some default previous data
		ServerNetwork::previousClientData[potentialClient] = PreviousData();
	}

	// broadcast message saying all client connections have been accepted.
	std::cout << "All clients connected!" << std::endl;

	return clientIDs;
}

std::vector<std::vector<NetworkResponse>> ServerNetwork::selectClients()
{
	std::vector<std::vector<NetworkResponse>> responses;
	responses.resize(clients.size()); // size it to how it needs to be

	// create and zero file descriptor set for select population
	fd_set readfds;
	FD_ZERO(&readfds);

	// add clients to fd_set
	for (int clientSock : clients)
	{
		FD_SET(clientSock, &readfds);
	}

	// 1ms timeout for detecting messages
	// we dont got time for waiting!
	timeval tv;
	tv.tv_usec = 1000;

	/**
	 * Realtalk: is doing select necessary? As the sockets are all nonblocking what's
	 * the harm in just polling *all* of them to see if they have data?
	 */
	int selResult = select(clients.back() + 1, &readfds, NULL, NULL, &tv);
	if (selResult < 0)
	{
		printf("select failed with error: %d\n", WSAGetLastError());
# ifdef _EXCEPTIONAL
		std::string message = "recv failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif
		WSACleanup();
	}

	for (int client_id = 0; client_id < clients.size(); ++client_id){
		int relevantSocket = clients[client_id];

		if (!FD_ISSET(relevantSocket, &readfds)){
			//std::cerr << client_id << " with socket " << relevantSocket << "is not ready to be read" << std::endl;
		}
		std::vector<NetworkResponse> relevantResponse = ServerNetwork::handleClient(relevantSocket);
		responses[client_id] = relevantResponse;
	}

	return responses;
}

void ServerNetwork::broadcastBytes(std::vector<char> bytes, int msgType, int id)
{
	for (int clientID = 0; clientID < clients.size(); clientID++)
	{
		ServerNetwork::sendBytes(clientID, bytes, msgType, id);
	}
}

void ServerNetwork::sendBytes(int clientID, std::vector<char> bytes, int msgType, int id)
{
	int clientSock = ServerNetwork::clients[clientID];

	// insert encoded type
	std::vector<char> encodedMsg = encodeMessage(bytes, msgType, id);

	int iSendResult = send(clientSock, encodedMsg.data(), encodedMsg.size(), 0);
	if (iSendResult == SOCKET_ERROR) {
		int wsaLastError = WSAGetLastError();
		if (wsaLastError != WSAEWOULDBLOCK)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(clientSock);
# ifdef _EXCEPTIONAL
			std::string message = "send failed with error: ";
			message += WSAGetLastError();
			throw new std::runtime_error(message);
# endif

			WSACleanup();
		}
		return;
	}
	return;
}

int ServerNetwork::acceptTCPConnection(int listenSocket) {
	// Accept a client socket
	int clientSocket;
	clientSocket = accept(listenSocket, NULL, NULL);

	if (clientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
# ifdef _EXCEPTIONAL
		std::string message = "accept failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

		WSACleanup();
		return -1;
	}

	const char shouldNoDelay = 1;
	setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &shouldNoDelay, sizeof(shouldNoDelay));

	std::cout << "accepted client!" << std::endl;

	return clientSocket;
}

int ServerNetwork::setupSocket(std::string port) 
{
	WSADATA wsaData;

	int iResult;

	int ListenSocket = INVALID_SOCKET;
	int ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
# ifdef _EXCEPTIONAL
		std::string message = "WSAStartup failed with error: ";
		message += iResult;
		throw new std::runtime_error(message);
# endif
		return -1;
	}
	ZeroMemory(&hints, sizeof(hints));

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

		WSACleanup();
		return -1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
# ifdef _EXCEPTIONAL
		std::string message = "socket failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

		WSACleanup();
		return -1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
# ifdef _EXCEPTIONAL
		std::string message = "bind failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

		WSACleanup();
		return -1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
# ifdef _EXCEPTIONAL
		std::string message = "listen failed with error: ";
		message += WSAGetLastError();
		throw new std::runtime_error(message);
# endif

		WSACleanup();
		return -1;
	}
	return ListenSocket;
}
