#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <vector>

class ServerManager
{
private:
	static std::vector<int> clientIDs;

public:
	ServerManager() {};
	~ServerManager() {};

	static std::vector<int> initialize(std::string port, int numberOfClients);

	static void sendMessages();
	static void receiveMessages();
};

#endif // SERVER_MANAGER_H