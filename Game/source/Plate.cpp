#include "Plate.h"
#include "Config.h"

#include <iostream>

Plate::Plate(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName)
	: isNotColliding(true), isColliding(false)
{
	// get around the fact that blender names things xxxx.001, xxxx.002 etc
	int size = 0;
	if (tokens[tokens.size() - 1].size() == 3) {
		size = tokens.size() - 1;
	}
	else {
		size = tokens.size();
	}
	for (int i = 1; i < size; i += 3)
	{
		int targetID = std::stoi(tokens[i + 0]);
		TriggerType triggerType = strToTriggerType(tokens[i + 1]);
		int activatorID = std::stoi(tokens[i + 2]);

		this->addConnection(Connection(idToTargets.at(groupName + std::to_string(targetID)), triggerType));
	}
}

Plate::~Plate()
{
}

void Plate::create()
{
	ConfigFile file("config/sounds.ini");
	auto colNode = gameObject->findChildByNameContains("Colliders");
	auto boxCollider = colNode != nullptr ? colNode->findChildByNameContains("BoxCollider") : nullptr;
	stepOn = Sound::affixSoundToDummy(boxCollider, new Sound("plate_stepOn", false, false, file.getFloat("plate_stepOn", "volume"), true));
	stepOff = Sound::affixSoundToDummy(boxCollider, new Sound("plate_stepOff", false, false, file.getFloat("plate_stepOn", "volume"), true));
}

void Plate::fixedUpdate()
{
	// we're on the edge!!
	if (!isColliding && !isNotColliding)
	{
		isNotColliding = true;
		stepOff->play();
		deactivate();
	}

	// we're on the edge!!
	if (isColliding && isNotColliding)
	{
		isNotColliding = false;
		activate();
		stepOn->play();
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