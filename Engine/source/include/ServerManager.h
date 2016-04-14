#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

class ServerManager
{
public:
	ServerManager() {};
	~ServerManager() {};

	static void sendMessages();
	static void receiveMessages();
};

#endif // SERVER_MANAGER_H