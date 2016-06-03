#include "GodSummoner.h"

#include <iostream>
#include "GameObject.h"
#include "Timer.h"

GodSummoner::GodSummoner(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(1);
	(*idToTarget)[groupName + std::to_string(targetID)] = this;
}


GodSummoner::~GodSummoner()
{
}

void GodSummoner::create()
{
}

void GodSummoner::fixedUpdate()
{
	float deltaTime = Timer::fixedTimestep;

	if (isActivated())
	{
		gameObject->transform.rotate(glm::quat(glm::vec3(0.0f, 0.0f, deltaTime * 0.25f)));
	}
}
