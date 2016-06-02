#include "KeyTarget.h"
#include "KeyActivator.h"
#include "Timer.h"
#include <iostream>

KeyTarget::KeyTarget()
{
}

KeyTarget::KeyTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName)
{
	if (tokens.size() > 4) { // Key is both a target and activator, so get info for target
		int targetID = std::stoi(tokens[4]);
		int threshold = std::stoi(tokens[5]);

		setThreshold(threshold);
		(*idToTarget)[groupName + std::to_string(targetID)] = this;
	}
	else { // key is only activator so allow it to be picked up
		canBePickedUp = true;

		// keys not in chests are never "activated". This prevents movement animation
		applyTrigger(TriggerType::NEGATIVE);
	}
}

KeyTarget::~KeyTarget()
{
}

void KeyTarget::create()
{
	initPosition = gameObject->transform.getPosition();
}

void KeyTarget::fixedUpdate()
{
	if (!pickedUp) {

		float deltaTime = Timer::fixedTimestep;

		openness += (deltaTime) * (isActivated() ? 1 : -1);
		openness = std::min(1.1f, openness);
		openness = std::max(-0.1f, openness);

		if (openness > 1.0f || openness < 0.0f) 
			return;

		gameObject->transform.setPosition(initPosition + glm::vec3(0, 0, 1) * openness * 1.2f);
	}
}
