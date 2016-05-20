#include "FixedKeyTarget.h"
#include "KeyActivator.h"
#include "Timer.h"
#include <iostream>

FixedKeyTarget::FixedKeyTarget()
{
}

FixedKeyTarget::FixedKeyTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	if (tokens.size() > 4) { // Key is both a target and activator, so get info for target
		int targetID = std::stoi(tokens[4]);
		int threshold = std::stoi(tokens[5]);

		setThreshold(threshold);
		(*idToTarget)[targetID] = this;
	}
	else { // key is only activator so allow it to be picked up
		canBePickedUp = true;

		// keys not in chests are never "activated". This prevents movement animation
		applyTrigger(TriggerType::NEGATIVE); 
	}
}

FixedKeyTarget::~FixedKeyTarget()
{
}

void FixedKeyTarget::create()
{
	initPosition = gameObject->transform.getPosition();
}

void FixedKeyTarget::fixedUpdate()
{

	if (isActivated() && !pickedUp) {
		float deltaTime = Timer::fixedTimestep;

		openness += (deltaTime) * (isActivated() ? 1 : -1);
		openness = std::min(1.0f, openness);
		openness = std::max(0.0f, openness);

		gameObject->transform.setPosition(initPosition + glm::vec3(0, 0, 1) * openness * 1.2f);
	}
}
