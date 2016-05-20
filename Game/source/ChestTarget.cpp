#include "ChestTarget.h"
#include "ChestActivator.h"
#include <iostream>
#include "Timer.h"

ChestTarget::ChestTarget()
{
}

ChestTarget::ChestTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	int targetID = std::stoi(tokens[4]);
	int threshold = std::stoi(tokens[5]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;

}

ChestTarget::~ChestTarget()
{
}

void ChestTarget::create()
{
	initPosition = gameObject->transform.children[0]->getPosition();
}

void ChestTarget::fixedUpdate()
{

	float deltaTime = Timer::fixedTimestep;

	openness += (deltaTime) * (isActivated() ? 1 : -1);
	openness = std::min(1.0f, openness);
	openness = std::max(0.0f, openness);

	gameObject->transform.children[0]->setPosition(initPosition + glm::vec3(0, 0, 1) * openness * 2.0f);
	gameObject->getComponent<ChestActivator>()->trigger(isActivated());
	
}
