#include "Plate.h"

#include <iostream>

Plate::Plate(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets)
	: isNotColliding(true), isColliding(false)
{
	for (int i = 1; i < tokens.size(); i += 3)
	{
		int targetID = std::stoi(tokens[i + 0]);
		TriggerType triggerType = strToTriggerType(tokens[i + 1]);
		int activatorID = std::stoi(tokens[i + 2]);

		this->addConnection(Connection(idToTargets.at(targetID), triggerType));
	}
}

Plate::~Plate()
{
}

void Plate::fixedUpdate()
{
	// we're on the edge!!
	if (!isColliding && !isNotColliding)
	{
		isNotColliding = true;
		deactivate();
	}

	// we're on the edge!!
	if (isColliding && isNotColliding)
	{
		isNotColliding = false;
		activate();
		std::cout << "activated plate" << std::endl;
	}

	isColliding = false;
}

void Plate::collisionEnter(GameObject *other)
{
}

void Plate::collisionStay(GameObject *other)
{
	isColliding = true;
}