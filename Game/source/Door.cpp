#include "Door.h"

#include "Timer.h"
#include <iostream>

Door::Door() {}

Door::Door(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, DoorMovement moveDirection, std::string groupName, bool canReopen)
	: moveDirection(moveDirection), locked(true)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);
	reopen = canReopen;

	setThreshold(threshold);
	(*idToTarget)[groupName + std::to_string(targetID)] = this;
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
	case BOTH:
		return glm::vec3(1, 0, 0);
	}
}

void Door::create()
{
	// Seems like doors in master.blend have positions that work well enough
	ConfigFile file("config/sounds.ini");
	float vol = file.getFloat("doorunlock", "volume");
	unlockSound = Sound::affixSoundToDummy(gameObject, new Sound("doorunlock", false, false, vol, true, Sound::NO_DAMPEN_CHANNEL));
	initPosit = gameObject->transform.getPosition();
}

void Door::fixedUpdate()
{
	float deltaTime = Timer::fixedTimestep;

	if (reopen) {
		openness += (deltaTime) * (isActivated() ? 1 : -1);
	}
	else openness += (deltaTime) * (isActivated() ? 1 : 0);

	openness = std::min(0.6f, openness);
	openness = std::max(-0.1f, openness);
	
	if (locked && isActivated()) {
		locked = false;
		unlockSound->play();
	}
	
	if (openness < 0.0 || openness > 0.5) return; 

	if (moveDirection == BOTH) {
		gameObject->findChildByNameContains("Door_Left")->transform.setPosition(moveDirectionVec() * -openness * 2.5f);
		gameObject->findChildByNameContains("Door_Right")->transform.setPosition(moveDirectionVec() * openness * 2.5f);
	}
	else gameObject->transform.setPosition(initPosit + moveDirectionVec() * openness * 2.5f);
}
