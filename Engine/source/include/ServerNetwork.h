#pragma once

#include <string>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class ServerNetwork
{

private:
	static int clientSocket;
	static int listenSocket;
	static std::string port;

	static int setupSocket(std::string port);
	static int acceptTCPConnection(int listenSocket);
	static void handleClient(int clientSocket, void * msg, int * msgType);

	//std::string encodeMessage(std::string msg);

public:
	ServerNetwork() {}
	~ServerNetwork() { ServerNetwork::closeConnection(); }
	
	static void setup(std::string port);
	static void closeConnection();

	static void start();
	static void handleClient(void * msg, int * msgType);
	static int sendMessage(std::string message);

};

