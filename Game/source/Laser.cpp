#include "Laser.h"
#include "Input.h"
#include "FPSMovement.h"
#include <iostream>

Laser::Laser()
{
}

Laser::Laser(std::vector<std::string> tokens)
{
	//TODO parse(tokens);
}

Laser::Laser()
{
}

Laser::Laser(std::vector<std::string> tokens)
{
	//TODO parse(tokens);
}

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

FixedLaser::FixedLaser(std::vector<std::string> tokens)
{
	//TODO parse(tokens);
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

}

void FixedLaser::collisionStay(GameObject *other)
{
	// respawns player at their original starting point.
	if (!areLasersOff) {
		GameObject * go = other->transform.getParent()->gameObject;
		FPSMovement * fps = go->getComponent<FPSMovement>();
		fps->respawn();
	}
}
