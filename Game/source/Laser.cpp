#include "Laser.h"
#include "Input.h"
#include "FPSMovement.h"
#include <iostream>
#include "Config.h"


Laser::Laser()
{
}

Laser::Laser(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, bool isFixed, std::string groupName)
	: isFixed(isFixed), areLasersOff(false)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[groupName + std::to_string(targetID)] = this;
}

Laser::~Laser()
{
}

void Laser::fixedUpdate()
{
	// Turn The Lasers off for the first time
	if (isActivated() && !areLasersOff)
	{
		areLasersOff = true;
		gameObject->setVisible(false);
		for (auto hum : passiveHum) {
			hum->pause();
		}
	}
	// Lasers are back on
	else if (!isActivated() && areLasersOff && !isFixed)
	{
		areLasersOff = false;
		gameObject->setVisible(true);

		for (auto hum : passiveHum) {
			hum->play();
		}
	}
}


void Laser::collisionStay(GameObject *other)
{
	// respawns player at their original starting point.
	if (!areLasersOff) 
	{
		GameObject * go = other->transform.getParent()->gameObject;
		FPSMovement * fps = go->getComponent<FPSMovement>();
		if (fps != nullptr) {
			fps->respawn();
		}
	}
}

void Laser::create()
{
	auto colNode = gameObject->findChildByNameContains("Colliders");
	ConfigFile file("config/sounds.ini");
	if (colNode) {
		for (auto collider : colNode->transform.children) {
			if (collider->gameObject) {
				passiveHum.push_back(
					Sound::affixSoundToDummy(
						collider->gameObject, 
						new Sound("ff_passive", true, true, file.getFloat("laser_passive", "volume"), true)));
			}
		}
	}
}
