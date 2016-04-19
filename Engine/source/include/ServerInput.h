#ifndef SERVER_INPUT_H
#define SERVER_INPUT_H

#include "ForwardDecs.h"
#include <unordered_map>
#include "NetworkStruct.h"

#include "Input.h"

/**
 * A proxy input class that aims to mirror Input.h in interface, while using server
 * data as its source.
 *
 * Functions need to be written as different sources of input are required.
 */
struct MovementData {
	float yaw, pitch, roll;
	glm::vec2 mousePos;
};

class ServerInput
{
private:
	static std::unordered_map<int, MovementData> clientMovementData;

public:
	ServerInput();
	~ServerInput();

	static glm::vec2 mousePosition(int clientId);
	static float getAxis(std::string name, int clientId);

	static void deserializeAndApply(std::vector<char> bytes, int clientId);
};

#endif SERVER_INPUT_H

