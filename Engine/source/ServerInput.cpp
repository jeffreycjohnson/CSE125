#include "ServerInput.h"

#include <iostream>
#include <sstream>

float ServerInput::yaw = 0.0f, ServerInput::pitch = 0.0f, ServerInput::roll = 0.0f;
glm::vec2 ServerInput::mousePos = glm::vec2(0.0f, 0.0f);

ServerInput::~ServerInput() {}

glm::vec2 ServerInput::mousePosition()
{
	return ServerInput::mousePos;
}

float ServerInput::getAxis(std::string name)
{
	if (name == "yaw") return yaw;
	if (name == "pitch") return pitch;
	if (name == "roll") return roll;

	std::cerr << "Requesting invalid axis " << name << std::endl;
	return 0.0f;
}

void ServerInput::deserializeStringAndApply(std::string serialized)
{
	std::stringstream ss(serialized);

	ss >> ServerInput::yaw;
	ss.ignore(); // DEM SEMICOLONS
	ss >> ServerInput::pitch;
	ss.ignore();
	ss >> ServerInput::roll;
	ss.ignore();

	ss >> ServerInput::mousePos.x;
	ss.ignore();
	ss >> ServerInput::mousePos.y;
}