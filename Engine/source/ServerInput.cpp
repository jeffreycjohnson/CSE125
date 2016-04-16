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

	std::cerr << "Requesting invalid axis " << name << std::endl;
	return 0.0f;
}

void ServerInput::deserializeStringAndApply(std::string serialized, int clientId)
{
	std::stringstream ss(serialized);

	ss >> ServerInput::clientMovementData[clientId].yaw;
	ss.ignore(); // DEM SEMICOLONS
	ss >> ServerInput::clientMovementData[clientId].pitch;
	ss.ignore();
	ss >> ServerInput::clientMovementData[clientId].roll;
	ss.ignore();

	ss >> ServerInput::clientMovementData[clientId].mousePos.x;
	ss.ignore();
	ss >> ServerInput::clientMovementData[clientId].mousePos.y;
}

void ServerInput::deserializeAndApply(InputNetworkData serialized, int clientId)
{
	ServerInput::clientMovementData[clientId].yaw = serialized.yaw;
	ServerInput::clientMovementData[clientId].pitch = serialized.pitch;
	ServerInput::clientMovementData[clientId].roll = serialized.roll;

	ServerInput::clientMovementData[clientId].mousePos.x = serialized.mouseX;
	ServerInput::clientMovementData[clientId].mousePos.y = serialized.mouseY;
}
