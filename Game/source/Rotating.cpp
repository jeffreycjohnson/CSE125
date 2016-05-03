#include "Rotating.h"

#include "Timer.h"

Rotating::Rotating()
{
}

Rotating::Rotating(int activationThreshold)
	: Target(activationThreshold)
{
}


Rotating::~Rotating()
{
}

void Rotating::fixedUpdate()
{
	float deltaTime = Timer::fixedTimestep;

	if (isActivated())
	{
		gameObject->transform.rotate(glm::quat(glm::vec3(deltaTime * 2.0f, deltaTime * 2.5f, deltaTime * 2.1f)));
	}
}
