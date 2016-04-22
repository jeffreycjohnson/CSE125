#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "ForwardDecs.h"
#include <vector>

class ClientManager
{
	static bool allClientsConnected;
	static std::vector<int> clientIDs;
	static int lastObjectCreated;
public:
	ClientManager() {};
	~ClientManager() {};

	static int myClientID;

	static const std::vector<int>& initialize(std::string serverIP, std::string port);

	static void sendMessages();
	static void receiveMessages();
};

#endif // CLIENT_MANAGER_H