#include "ClientLoop.h"

#include <sstream>
#include <iostream>
#include "GameObject.h"

ClientLoop::ClientLoop(std::string serverIP, std::string port) : serverIP(serverIP), port(port)
{
}

ClientLoop::~ClientLoop()
{
}

void ClientLoop::create()
{
	// ClientNetwork::SetupTCPConnection(serverIP, port);
}

void ClientLoop::update(float dt)
{
}
