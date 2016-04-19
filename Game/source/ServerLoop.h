#ifndef SERVERLOOP_H
#define SERVERLOOP_H

#include "Component.h"
#include "ServerNetwork.h"

#include <iostream>

class ServerLoop :
	public Component
{

public:
	ServerLoop(std::string port);
	~ServerLoop() { std::cout << "SERVER LOOP DYING" << std::endl; };

	void create() override;
	void fixedUpdate() override;
};

#endif // SERVERLOOP_H