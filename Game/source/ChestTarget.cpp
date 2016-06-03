#include "ChestTarget.h"
#include "ChestActivator.h"
#include <iostream>
#include "Timer.h"

ChestTarget::ChestTarget()
{
}

ChestTarget::ChestTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName)
{
	int targetID = std::stoi(tokens[4]);
	int threshold = std::stoi(tokens[5]);

	setThreshold(threshold);
	(*idToTarget)[groupName + std::to_string(targetID)] = this;

}

ChestTarget::~ChestTarget()
{
}

void ChestTarget::create()
{
	initPosition = gameObject->transform.children[0]->getPosition();

	gameObject->getComponent<ChestActivator>()->trigger(false);
}

void ChestTarget::fixedUpdate()
{

	float deltaTime = Timer::fixedTimestep;

	openness += (deltaTime) * (isActivated() ? 1 : -1);
	openness = std::min(1.1f, openness);
	openness = std::max(-0.1f, openness);

	if (openness > 1.0f || openness < 0.0f) return;

	gameObject->transform.children[0]->setPosition(initPosition + glm::vec3(0, 0, 1) * openness * 2.0f);
	gameObject->getComponent<ChestActivator>()->trigger(isActivated());
	
}
