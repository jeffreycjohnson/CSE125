#ifndef SERVER_INPUT_H
#define SERVER_INPUT_H

#include "ForwardDecs.h"
#include <unordered_map>

#include "Input.h"

/**
 * A proxy input class that aims to mirror Input.h in interface, while using server
 * data as its source.
 *
 * Functions need to be written as different sources of input are required.
 */
class ServerInput
{
private:
	static float yaw, pitch, roll;
	static glm::vec2 mousePos;

public:
	ServerInput();
	~ServerInput();

	static glm::vec2 mousePosition();
	static float getAxis(std::string name);

	static void deserializeStringAndApply(std::string serialized);
};

#endif SERVER_INPUT_H

