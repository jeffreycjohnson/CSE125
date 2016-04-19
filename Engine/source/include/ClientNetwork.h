#pragma once
#include <string>
#include <vector>

class ClientNetwork
{
private:
	static std::string serverIp;
	static std::string port;
	static bool ConnectionEstablished;
	static int ConnectSocket;

public:
	ClientNetwork() {}
	~ClientNetwork() { ClientNetwork::CloseConnection(); }
	//Establishes a Connection based on the ip and port
	//Returns 1 if failure

	static int SetupTCPConnection(std::string serverip, std::string port);

	static char decodeMessage(std::string message);
	//Requirements, need connection to be established... What should the data type be TODO:
	static int sendMessage(void * message, int msgType);
	static int sendBytes(std::vector<char> bytes, int msgType);

	//Returns string with recieved
	static std::vector<char> receiveMessage(int * msgType);

	//Manual Shutdown
	static int CloseConnection();

	//Status
	static void GetStatus(std::string header = std::string());
};

