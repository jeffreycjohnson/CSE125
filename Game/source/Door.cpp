#include "Door.h"

#include "Timer.h"
#include <iostream>

Door::Door() {}

Door::Door(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget, DoorMovement moveDirection)
	: moveDirection(moveDirection)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;
}

Door::~Door()
{
}

glm::vec3 Door::moveDirectionVec()
{
	switch (moveDirection)
	{
	case LEFT:
		return glm::vec3(-1, 0, 0);
	case RIGHT:
		return glm::vec3(1, 0, 0);
	case UP:
		return glm::vec3(0, 0, 1);
	case DOWN:
		return glm::vec3(0, 0, -1);
	}
}

void Door::create()
{
	initPosit = gameObject->transform.getPosition();
}

void Door::fixedUpdate()
{
	float deltaTime = Timer::fixedTimestep;

	openness += (deltaTime) * (isActivated() ? 1 : -1);
	openness = std::min(1.0f, openness);
	openness = std::max(0.0f, openness);

	gameObject->transform.setPosition(initPosit + moveDirectionVec() * openness * 2.5f);
}
