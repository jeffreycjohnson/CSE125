#include "GodSummoner.h"

#include <iostream>
#include "GameObject.h"
#include "Timer.h"

GodSummoner::GodSummoner(GameObject *toRotate)
	: toRotate(toRotate)
{
}


GodSummoner::~GodSummoner()
{
}

void GodSummoner::fixedUpdate()
{
	float deltaTime = Timer::fixedTimestep;
	if (rotating)
	{
		toRotate->transform.rotate(glm::quat(glm::vec3(deltaTime * 2.0f, deltaTime * 2.5f, deltaTime * 2.1f)));
		std::cerr << toRotate->transform.getRotation().w << ", " << toRotate->transform.getRotation().x << ", " << toRotate->transform.getRotation().y << ", " << toRotate->transform.getRotation().z << ", " << std::endl;
	}

	rotating = false;
}

void GodSummoner::collisionEnter(GameObject * other)
{
	std::cout << "start colliding" << std::endl;
}

void GodSummoner::collisionStay(GameObject * other)
{
	std::cout << other->getName() << std::endl;
	rotating = true;
}

void GodSummoner::collisionExit(GameObject * other)
{
	std::cout << "end colliding" << std::endl;
}

void GodSummoner::setTarget(GameObject * target)
{
	if (target != nullptr)
	{
		this->toRotate = target;
	}
}
