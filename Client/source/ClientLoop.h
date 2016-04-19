#ifndef CLIENTLOOP_H
#define CLIENTLOOP_H

#include "Component.h"

#include "ClientNetwork.h"

class ClientLoop :
	public Component
{
private:
	std::string serverIP;
	std::string port;
public:
	ClientLoop(std::string serverIP, std::string port);
	~ClientLoop();

	void create() override;
	void update(float dt) override;
};

#endif // CLIENTLOOP_H