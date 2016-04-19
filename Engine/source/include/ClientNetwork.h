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

	static char decodeMessage(std::string message);
	//Requirements, need connection to be established... What should the data type be TODO:
	static int sendMessage(void * message, int msgType);
	static int sendBytes(std::vector<char> bytes, int msgType);

	//Returns string with recieved
	static std::vector<NetworkResponse> receiveMessages();

	//Manual Shutdown
	static int closeConnection();

	//Status
	static void getStatus(std::string header = std::string());
};

