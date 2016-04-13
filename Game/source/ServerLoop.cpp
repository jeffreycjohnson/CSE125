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
	float dt = Timer::fixedTimestep;

	// 1. read msgs from client
	std::string msg = ServerNetwork::handleClient();
	if (msg != "")
	{
		std::stringstream msgStrm(msg);

		float w, x, y, z;

		msgStrm >> w;
		msgStrm.ignore();
		msgStrm >> x;
		msgStrm.ignore();
		msgStrm >> y;
		msgStrm.ignore();
		msgStrm >> z;

		//std::cout << w << "," << x << "," << y << "," << z << std::endl;

		glm::quat q(w, x, y, z);
		GameObject *player = GameObject::FindByName("player");

		if (q != glm::quat(1, 0, 0, 0))
		{
			player->transform.setRotate(q);
		}

		glm::quat rotation = player->transform.getRotation();

		std::stringstream qbuild;
		qbuild << rotation.w << "," << rotation.x << "," << rotation.y << "," << rotation.z;

		std::string qs = qbuild.str();
		ServerNetwork::sendMessage(qs);
	}
}
