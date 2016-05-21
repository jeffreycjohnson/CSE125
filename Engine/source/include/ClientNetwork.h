#pragma once

#include <string>
#include <vector>

#include "NetworkUtility.h"

class ClientNetwork
{
private:
	static std::string serverIp;
	static std::string port;
	static bool ConnectionEstablished;
	static int ConnectSocket;

	static PreviousData previousServerData;
public:
	ClientNetwork() {}
	~ClientNetwork() { ClientNetwork::closeConnection(); }
	//Establishes a Connection based on the ip and port
	//Returns 1 if failure

	static int setup(std::string serverip, std::string port);
	static int sendBytes(std::vector<char> bytes, int msgType, int id);

	//Returns string with recieved
	static std::vector<NetworkResponse> receiveMessages();

	//Manual Shutdown
	static int closeConnection();

	//Status
	static void getStatus(std::string header = std::string());
	static bool isConnected();
};

