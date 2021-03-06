#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "NetworkStruct.h"
#include "NetworkStats.h"
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

//bool init_log = false;
//std::vector<std::vector<int>> logger;

void ServerNetwork::setup(std::string port)
{
	ServerNetwork::port = port;
	std::cout << "Starting server on port " << port << std::endl;
	ServerNetwork::listenSocket = setupSocket(port);

	if (listenSocket == -1) {
		std::cerr << "Error in setupSocket()" << std::endl;

#ifdef _EXCEPTIONAL
		FATAL("Error in setupSocket()");
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
			FATAL(message.c_str());
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
	int totalBytesRecvd = 0;

	std::vector<NetworkResponse> msgs;
	std::vector<char> currMsg;

	// non-blocking mode is enabled.
	u_long iMode = 1;
	ioctlsocket(clientSocket, FIONBIO, &iMode);

	char recvbuf[DEFAULT_BUFLEN];
	PreviousData& prevData = ServerNetwork::previousClientData[clientSocket];

	if (prevData.length != 0)
	{
		memcpy(recvbuf, prevData.buffer, DEFAULT_BUFLEN);
		totalBytesRecvd += prevData.length;

		memset(prevData.buffer, 0, DEFAULT_BUFLEN);
		prevData.length = 0;
	}

	do {
		int bytesLastRecv = recv(clientSocket, recvbuf + totalBytesRecvd, DEFAULT_BUFLEN - totalBytesRecvd - 1 , 0);

		int nError = WSAGetLastError();
		if (nError == WSAEWOULDBLOCK) {
			break;
		}

		int totalBytesProcd = 0;
		if (bytesLastRecv > 0) {
			totalBytesRecvd += bytesLastRecv;
			int msgLength = 0;
			totalBytesProcd = 0;

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
				int contentLength = -1;
				currMsg = decodeMessage(recvbuf + totalBytesProcd, DEFAULT_BUFLEN, &msgType, &msgId, &contentLength);
				totalBytesProcd += contentLength;

				NetworkResponse response(msgType, msgId, currMsg);
				msgs.push_back(response);
			}
			while (totalBytesRecvd > totalBytesProcd);
			
			//std::cerr << clientSocket << ": We found " << msgs.size() << " messages" << std::endl;
			break;
		}
		else if (bytesLastRecv == 0) 
		{
			if (totalBytesProcd != totalBytesRecvd)
			{
				std::cerr << "THROWING AWAY BYTES" << std::endl;
			}

			break;
		}
		else  
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);

# ifdef _EXCEPTIONAL
			std::string message = "recv failed with error: ";
			message += WSAGetLastError();
			FATAL(message.c_str());
# endif
			WSACleanup();
			return msgs;
		}

	} while (1);

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
	NetworkStats::registerClients(clientIDs);

	return clientIDs;
}

std::vector<std::vector<NetworkResponse>> ServerNetwork::selectClients()
{
	std::vector<std::vector<NetworkResponse>> responses;
	responses.resize(clients.size()); // size it to how it needs to be

	for (int client_id = 0; client_id < clients.size(); ++client_id){
		int relevantSocket = clients[client_id];

		std::vector<NetworkResponse> relevantResponse = ServerNetwork::handleClient(relevantSocket);
		responses[client_id] = relevantResponse;
	}

	return responses;
}

void ServerNetwork::broadcastBytes(const std::vector<char> &bytes, int msgType, int id)
{
	for (int clientID = 0; clientID < clients.size(); clientID++)
	{
		ServerNetwork::sendBytes(clientID, bytes, msgType, id);
	}
}

void ServerNetwork::sendBytes(int clientID, const std::vector<char> &bytes, int msgType, int id)
{
	int clientSock = ServerNetwork::clients[clientID];
	NetworkStats::registerMsgType(clientID, msgType);
	// insert encoded type
	int checksum = 0;
	for (char c : bytes) {
		checksum += c;
	}

	std::vector<char> encodedMsg = encodeMessage(bytes, msgType, id, checksum);
	ServerNetwork::sendBytes(clientID, encodedMsg);

	return;
}

void ServerNetwork::sendBytes(int clientID, const std::vector<char>& bytes)
{
	int clientSock = ServerNetwork::clients[clientID];

	//std::cout << "Sending " << encodedMsg.size() << " bytes to client " << clientID  << " with checksum " << checksum << std::endl;
	int iSendResult = send(clientSock, bytes.data(), bytes.size(), 0);
	if (iSendResult == SOCKET_ERROR) {
		int wsaLastError = WSAGetLastError();
		if (wsaLastError != WSAEWOULDBLOCK)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(clientSock);
# ifdef _EXCEPTIONAL
			std::string message = "send failed with error: ";
			message += WSAGetLastError();
			FATAL(message.c_str());
# endif

			WSACleanup();
		}
		return;
	}
	NetworkStats::registerBytesSent(clientID, bytes.size());
}

void ServerNetwork::sendMessages(int clientID, const std::vector<NetworkResponse>& messages)
{
	int clientSock = ServerNetwork::clients[clientID];
	std::vector<char> bytes;

	for (auto& msg : messages)
	{
		NetworkStats::registerMsgType(clientID, msg.messageType);
		// insert encoded type
		int checksum = 0;
		for (char c : msg.body) {
			checksum += c;
		}

		std::vector<char> encodedMsg = encodeMessage(msg.body, msg.messageType, msg.id, checksum);
		bytes.insert(bytes.end(), encodedMsg.begin(), encodedMsg.end());
	}

	ServerNetwork::sendBytes(clientID, bytes);
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
		FATAL(message.c_str());
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
		FATAL(message.c_str());
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
		FATAL(message.c_str());
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
		FATAL(message.c_str());
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
		FATAL(message.c_str());
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
		FATAL(message.c_str());
# endif

		WSACleanup();
		return -1;
	}
	return ListenSocket;
}
