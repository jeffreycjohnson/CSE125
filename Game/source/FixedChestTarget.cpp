#include "FixedChestTarget.h"
#include "ChestActivator.h"
#include <iostream>
#include "Timer.h"

FixedChestTarget::FixedChestTarget()
{
}

FixedChestTarget::FixedChestTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	int targetID = std::stoi(tokens[4]);
	int threshold = std::stoi(tokens[5]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;

}

FixedChestTarget::~FixedChestTarget()
{
}

void FixedChestTarget::create()
{
	initPosition = gameObject->transform.children[0]->getPosition();
}

void FixedChestTarget::fixedUpdate()
{
	if (isActivated() || isOpened) {
		isOpened = true;
		float deltaTime = Timer::fixedTimestep;
		
		openness += (deltaTime) * (isOpened ? 1 : -1);
		openness = std::min(1.0f, openness);
		openness = std::max(0.0f, openness);

		gameObject->transform.children[0]->setPosition(initPosition + glm::vec3(0, 0, 1) * openness * 2.0f);
		gameObject->getComponent<ChestActivator>()->trigger();
	}
}
