#include "ServerInput.h"

#include <iostream>
#include <sstream>

std::unordered_map<int, MovementData> ServerInput::clientMovementData;

ServerInput::~ServerInput() {}

glm::vec2 ServerInput::mousePosition(int clientId)
{
	return ServerInput::clientMovementData[clientId].mousePos;
}

float ServerInput::getAxis(std::string name, int clientId)
{
	if (name == "yaw") return ServerInput::clientMovementData[clientId].yaw;
	if (name == "pitch") return ServerInput::clientMovementData[clientId].pitch;
	if (name == "roll") return ServerInput::clientMovementData[clientId].roll;
	if (name == "jump") return ServerInput::clientMovementData[clientId].jump;
	if (name == "respawn") return ServerInput::clientMovementData[clientId].respawn;

	if (name == "aim") return ServerInput::clientMovementData[clientId].aim;

	std::cerr << "Requesting invalid axis " << name << std::endl;
	return 0.0f;
}

void ServerInput::deserializeAndApply(std::vector<char> bytes, int clientId)
{
	InputNetworkData ind = *((InputNetworkData*)bytes.data());

	ServerInput::clientMovementData[clientId].yaw = ind.yaw;
	ServerInput::clientMovementData[clientId].pitch = ind.pitch;
	ServerInput::clientMovementData[clientId].roll = ind.roll;
	ServerInput::clientMovementData[clientId].jump = ind.jump;
	ServerInput::clientMovementData[clientId].respawn = ind.respawn;

	ServerInput::clientMovementData[clientId].mousePos.x = ind.mouseX;
	ServerInput::clientMovementData[clientId].mousePos.y = ind.mouseY;

	ServerInput::clientMovementData[clientId].aim = ind.aim;
}
