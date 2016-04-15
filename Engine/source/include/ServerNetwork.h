#pragma once

#include <string>
#include <vector>

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "27015"

struct NetworkResponse
{
	int messageType;
	std::vector<char> body;

	NetworkResponse(int messageType, std::vector<char> body) :
		messageType(messageType), body(body)
	{}

	~NetworkResponse() {}
};

class ServerNetwork
{

private:
	static int clientSocket;
	static int listenSocket;
	static std::string port;

	static char lastBuf[DEFAULT_BUFLEN];
	static int lastLen;

	static int setupSocket(std::string port);
	static int acceptTCPConnection(int listenSocket);
	static std::vector<NetworkResponse> handleClient(int clientSocket);

	//std::string encodeMessage(std::string msg);

public:
	ServerNetwork() {}
	~ServerNetwork() { ServerNetwork::closeConnection(); }
	
	static void setup(std::string port);
	static void closeConnection();

	static void start();
	static std::vector<NetworkResponse> handleClient();
	static int sendMessage(void * message, int msgType);

};

