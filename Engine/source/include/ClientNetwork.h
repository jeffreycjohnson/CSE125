#pragma once
#include <string>

class ClientNetwork
{
private:
	std::string serverIp;
	std::string port;
	bool ConnectionEstablished;
	int ConnectSocket;

	int SetupTCPConnection(std::string serverip, std::string port);

public:
	ClientNetwork();
	ClientNetwork(std::string ip, std::string port);
	//Establishes a Connection based on the ip and port
	//Returns 1 if failure

	char  decodeMessage(std::string message);
	//Requirements, need connection to be established... What should the data type be TODO:
	int sendMessage(std::string message);

	//Returns string with recieved
	std::string receiveMessage();

	//Manual Shutdown
	int CloseConnection();

	//Status
	void GetStatus(std::string header = std::string());

	~ClientNetwork();
};

