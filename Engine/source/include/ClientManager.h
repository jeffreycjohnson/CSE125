#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

class ClientManager
{
	static bool allClientsConnected;

public:
	ClientManager() {};
	~ClientManager() {};

	static void sendMessages();
	static void receiveMessages();
};

#endif // CLIENT_MANAGER_H