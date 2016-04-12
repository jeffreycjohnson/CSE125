#include "ClientLoop.h"

#include <sstream>
#include "GameObject.h"

ClientLoop::ClientLoop(std::string serverIP, std::string port) : serverIP(serverIP), port(port)
{
}

ClientLoop::~ClientLoop()
{
}

void ClientLoop::create()
{
	cliNet.SetupTCPConnection(serverIP, port);
}

void ClientLoop::update(float dt)
{
	GameObject *player = GameObject::FindByName("player");

	glm::quat rotation = player->transform.getRotation();

	std::stringstream qbuild;
	qbuild << rotation.w << "," << rotation.x << "," << rotation.y << "," << rotation.z;

	std::string qs = qbuild.str();
	cliNet.sendMessage(qs);

	cliNet.receiveMessage();
}
