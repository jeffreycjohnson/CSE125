#pragma once

#include <string>
#include <vector>

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
	static std::vector<char> handleClient(int clientSocket, int * msgType);

	//std::string encodeMessage(std::string msg);

public:
	ServerNetwork() {}
	~ServerNetwork() { ServerNetwork::closeConnection(); }
	
	static void setup(std::string port);
	static void closeConnection();

	static void start();
	static std::vector<char> handleClient(int * msgType);
	static int sendMessage(void * message, int msgType);

};

