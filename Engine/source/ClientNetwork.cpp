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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "NetworkStruct.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#ifdef __LINUX
#else
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "5000"

std::string ClientNetwork::serverIp;
std::string ClientNetwork::port;
bool ClientNetwork::ConnectionEstablished;
int ClientNetwork::ConnectSocket;

PreviousData ClientNetwork::previousServerData;

int ClientNetwork::closeConnection(){
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

	WSACleanup();
#endif

	ConnectionEstablished = false;
	ConnectSocket = INVALID_SOCKET;
	return closeError;
}

/**

	Sets up the ClientNetwork object.

	@param std::string ip: string containing the ip. Preferably, in the dotted quads form, e.g. "192.168.0.1"
	@param std::string port: string containing the port. Preferably, in an integer format, e.g. "42069"
	@return a result code
**/
int ClientNetwork::setup(std::string serverIp, std::string port){

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
# ifdef _EXCEPTIONAL
		std::string message = "WSAStartup failed with error: ";
		message += iResult;
		throw new std::runtime_error(message);
# endif

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

	// Attempt to connect to an address until one succeeds TODO: Handle Timeouts?
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
#ifdef __LINUX
#else
			printf("socket failed with error: %ld\n", WSAGetLastError());
# ifdef _EXCEPTIONAL
			std::string message = "socket failed with error: ";
			message += WSAGetLastError();
			throw new std::runtime_error(message);
# endif

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
# ifdef _EXCEPTIONAL
		std::string message = "Unable to connect to server!";
		throw new std::runtime_error(message);
# endif

#ifdef __LINUX
#else
		WSACleanup();
#endif
		return -1;
	}

	//Saving
	ClientNetwork::ConnectionEstablished = true;
	ClientNetwork::serverIp = serverIp;
	ClientNetwork::port = port;
	ClientNetwork::ConnectSocket = ConnectSocket;

	// get some dummy previous data
	previousServerData = PreviousData();

	return 0;
}

int ClientNetwork::sendMessage(void * message, int msgType) {
	char encodedMsg[DEFAULT_BUFLEN];
	int contentLength = encodeStruct(message, NetworkStruct::sizeOf(msgType), msgType, encodedMsg, DEFAULT_BUFLEN);

	if (contentLength == 0) return 1;

	//Need to Establish Connection
	if (!ConnectionEstablished){
		std::cerr << "Send Refused. Please Establish Connection" << std::endl;
		return 1;
	}
	int iSendResult = send(ClientNetwork::ConnectSocket, encodedMsg, contentLength, 0);
	if (iSendResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
		int wsaLastError = WSAGetLastError();
		if (wsaLastError != WSAEWOULDBLOCK)
		{
			std::cerr << "Send Failed with error: " << wsaLastError << std::endl;
			closesocket(ConnectSocket);
# ifdef _EXCEPTIONAL
			std::string message = "Send Failed with error: ";
			message += wsaLastError;
			throw new std::runtime_error(message);
# endif
			WSACleanup();
		}
#endif
		ClientNetwork::ConnectionEstablished = false;
		ClientNetwork::ConnectSocket = INVALID_SOCKET;
		return 1;
	}

	//Pound Define This
	// printf("Bytes Sent: %d\n", iSendResult);

	return 0;
}

int ClientNetwork::sendBytes(std::vector<char> bytes, int msgType)
{
	std::vector<char> encodedMsg = encodeMessage(bytes, msgType);

	//Need to Establish Connection
	if (!ConnectionEstablished){
		std::cerr << "Send Refused. Please Establish Connection" << std::endl;
		return 1;
	}
	std::cout << "Sending " << encodedMsg.size() << " bytes" << std::endl;
	int iSendResult = send(ClientNetwork::ConnectSocket, encodedMsg.data(), encodedMsg.size(), 0);
	if (iSendResult == SOCKET_ERROR) {
#ifdef __LINUX
#else
		int wsaLastError = WSAGetLastError();
		if (wsaLastError != WSAEWOULDBLOCK)
		{
			std::cerr << "Send Failed with error: " << wsaLastError << std::endl;
			closesocket(ConnectSocket);
# ifdef _EXCEPTIONAL
			std::string message = "Send Failed with error: ";
			message += wsaLastError;
			throw new std::runtime_error(message);
# endif
			WSACleanup();
		}
#endif
		ClientNetwork::ConnectionEstablished = false;
		ClientNetwork::ConnectSocket = INVALID_SOCKET;
		return 1;
	}
	
	return 0;
}

std::vector<NetworkResponse> ClientNetwork::receiveMessages()
{
	int iResult = 0;
	int totalBytesRecvd = 0;
	int contentLength = -1;
	int msgType = -1;

	std::vector<NetworkResponse> msgs;
	std::vector<char> msg;

	// enable nonblocking
	u_long iMode = 1;
	ioctlsocket(ConnectSocket, FIONBIO, &iMode);

	char recvbuf[DEFAULT_BUFLEN];
	memset(recvbuf, 0, DEFAULT_BUFLEN);
	
	// is there leftovers from last time?
	if (previousServerData.length != 0)
	{
		memcpy(recvbuf, previousServerData.buffer, DEFAULT_BUFLEN);
		totalBytesRecvd += previousServerData.length;
	}

	do
	{
		iResult = recv(ConnectSocket, recvbuf + totalBytesRecvd, DEFAULT_BUFLEN - totalBytesRecvd - 1, 0);

		// if there's nothing, just leave
		int nError = WSAGetLastError();
		if (nError == WSAEWOULDBLOCK) break;

		if (iResult > 0) // if there's data to fetch
		{
			totalBytesRecvd += iResult;
			int msgLength = 0;
			unsigned int totalBytesProcd = 0;

			do
			{
				// what do if we don't even have enough for the message length
				if ((totalBytesRecvd - totalBytesProcd) < sizeof(int))
				{
					memset(previousServerData.buffer, 0, DEFAULT_BUFLEN);
					memcpy(previousServerData.buffer, recvbuf + totalBytesProcd, DEFAULT_BUFLEN - totalBytesProcd);
					previousServerData.length = totalBytesRecvd - totalBytesProcd;

					break;
				}

				// grab the message length to see if we have enough
				msgLength = ntohl(*((int *)(recvbuf + totalBytesProcd)));

				// what do if we don't have enough bytes for the whole message
				if ((totalBytesRecvd - totalBytesProcd) < msgLength)
				{
					memset(previousServerData.buffer, 0, DEFAULT_BUFLEN);
					memcpy(previousServerData.buffer, recvbuf + totalBytesProcd, DEFAULT_BUFLEN - totalBytesProcd);
					previousServerData.length = totalBytesRecvd - totalBytesProcd;

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

			std::cout << ConnectSocket << ": We found " << msgs.size() << " messages" << std::endl;
			break;
		}
		else if (iResult == 0) // there's fookin nothin
		{
			break;
		}
		else // error
		{ 
#ifdef __LINUX
#else
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);

# ifdef _EXCEPTIONAL
			std::string message = "recv failed with error: ";
			message += WSAGetLastError();
			throw new std::runtime_error(message);
# endif
			WSACleanup();
#endif
			return msgs;
		}
	}
	while (iResult > 0);

	return msgs;
}

void ClientNetwork::getStatus(std::string header){
	std::cout << header;
	std::cout << "Ip and Port are " << ClientNetwork::serverIp << ":" << ClientNetwork::port << std::endl;
	std::cout << "Is the connection to the port established? " << ClientNetwork::ConnectionEstablished << std::endl;
	std::cout << "Connect Socket is " << ClientNetwork::ConnectSocket << std::endl;
	std::cout << std::endl;
}


