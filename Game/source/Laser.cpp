#include "Laser.h"

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