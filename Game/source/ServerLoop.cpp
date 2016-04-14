#include "ServerLoop.h"

#include <sstream>
#include <iostream>
#include "GameObject.h"
#include "Timer.h"

ServerLoop::ServerLoop(std::string port)
{
	ServerNetwork::setup(port);
}

void ServerLoop::create()
{
	ServerNetwork::start();
}

void ServerLoop::fixedUpdate()
{
	
}
