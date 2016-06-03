#include "Rotating.h"

#include "Timer.h"

Rotating::Rotating()
{
}

Rotating::Rotating(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[groupName+std::to_string(targetID)] = this;
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
