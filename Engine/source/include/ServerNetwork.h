#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "NetworkUtility.h"

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "27015"

typedef int clientID;

class ServerNetwork
{
private:
	static int listenSocket;
	static std::string port;
		
	static int setupSocket(std::string port);
	static int acceptTCPConnection(int listenSocket);
	static std::vector<NetworkResponse> handleClient(int clientSocket);

	// -- MULTICONN -- //

	// maps clientIDs (indices) to client socket descriptors
	static std::vector<int> clients;

	// maps client socket descriptors to any leftover data from the last round of selecting
	static std::unordered_map<int, PreviousData> previousClientData;

public:
	ServerNetwork() {}
	~ServerNetwork() { ServerNetwork::closeConnection(); }
	
	static void setup(std::string port);
	static void closeConnection();
	
	// -- MULTICONN -- //

	// returns IDs for the registered clients
	static std::vector<int> startMultiple(int numClients);

	// returns a mapping between client IDs and the messages received from them in the last tick
	static std::vector<std::vector<NetworkResponse>> selectClients();

	static void broadcastMessage(void *message, int msgType);
	static void sendMessage(int clientID, void *message, int msgType);

	static void broadcastBytes(std::vector<char> bytes, int msgType);
	static void sendBytes(int clientID, std::vector<char> bytes, int msgType);
};

