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
	cliNet.SetupTCPConnection(serverIP, port);
}

void ClientLoop::update(float dt)
{
	GameObject *player = GameObject::FindByName("player");

	glm::quat rotation = player->transform.getRotation();
	glm::quat noRotation;
	noRotation.w = 0;
	noRotation.x = 0;
	noRotation.y = 0;
	noRotation.z = 0;
	player->transform.setRotate(noRotation);

	std::stringstream qbuild;
	qbuild << rotation.w << "," << rotation.x << "," << rotation.y << "," << rotation.z;

	std::string qs = qbuild.str();
	cliNet.sendMessage(qs);

	std::string response = cliNet.receiveMessage();
	if (response != "") {
		std::stringstream msgStrm(response);

		float w, x, y, z;

		msgStrm >> w;
		msgStrm.ignore();
		msgStrm >> x;
		msgStrm.ignore();
		msgStrm >> y;
		msgStrm.ignore();
		msgStrm >> z;

		glm::quat serverRotation;
		serverRotation.w = w;
		serverRotation.x = x;
		serverRotation.y = y;
		serverRotation.z = z;

		std::cout << w << "," << x << "," << y << "," << z << std::endl;
		player->transform.setRotate(serverRotation);
	}
}
