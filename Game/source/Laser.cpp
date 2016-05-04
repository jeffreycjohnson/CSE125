#include "Laser.h"
#include "Input.h"
#include "FPSMovement.h"
#include <iostream>

Laser::Laser(int activationThreshold)
	: Target(activationThreshold)
{
}

Laser::~Laser()
{
}

void Laser::fixedUpdate()
{
}

FixedLaser::FixedLaser(int activationThreshold)
	: Target(activationThreshold), areLasersOff(false)
{
}

FixedLaser::~FixedLaser()
{
}

void FixedLaser::fixedUpdate()
{
	if (isActivated() && !areLasersOff)
	{
		areLasersOff = true;
		gameObject->setVisible(false);
	}
}

void FixedLaser::collisionEnter(GameObject *other)
{
	std::cout << "collision enter!" << std::endl;
}

void FixedLaser::collisionStay(GameObject *other)
{
	std::cout << "collision stay!" << std::endl;
	// respawns player at their original starting point.
	GameObject * go = other->transform.getParent()->gameObject;
	FPSMovement * fps = go->getComponent<FPSMovement>();
	fps->respawn();
}
