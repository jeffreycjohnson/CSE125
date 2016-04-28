#include "GodSummoner.h"

#include <iostream>
#include "GameObject.h"

GodSummoner::GodSummoner(GameObject *toRotate)
	: toRotate(toRotate)
{
}


GodSummoner::~GodSummoner()
{
}

void GodSummoner::update(float deltaTime)
{
	if (rotating)
	{
		toRotate->transform.rotate(glm::quat(glm::vec3(deltaTime * 2.0f, deltaTime * 2.5f, deltaTime * 2.1f)));
	}

	rotating = false;
}

void GodSummoner::collisionEnter(GameObject * other)
{
	std::cout << "start colliding" << std::endl;
}

void GodSummoner::collisionStay(GameObject * other)
{
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
