#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

class ClientManager
{
public:
	ClientManager() {};
	~ClientManager() {};

	static void sendMessages();
	static void receiveMessages();
};

#endif // CLIENT_MANAGER_H