#include "Laser.h"
#include "Input.h"
#include "FPSMovement.h"
#include <iostream>
#include "Sound.h"

Laser::Laser()
{
}

Laser::Laser(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
	: isFixed(true), areLasersOff(false)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;
}

Laser::~Laser()
{
}

void Laser::fixedUpdate()
{
	if (isActivated() && !areLasersOff)
	{
		areLasersOff = true;
		Sound * s = gameObject->getComponent<Sound>();
		gameObject->setVisible(false);
		s->play();
	}
	else if (!isActivated() && areLasersOff && !isFixed)
	{
		areLasersOff = false;
		gameObject->setVisible(true);
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
