#include "ActivatorRegistrator.h"

#include <iostream>
#include <string>

#include "GameObject.h"

ActivatorRegistrator::ActivatorRegistrator()
{
}


ActivatorRegistrator::~ActivatorRegistrator()
{
}

void ActivatorRegistrator::create()
{
	std::cout << "Registering activators..." << std::endl;

	// scan for activatables
	for (auto& obj : gameObject->transform.children)
	{
		if (obj->gameObject->getName().find("monkey_") != std::string::npos)
		{
			std::cout << "Found monkey: " << obj->gameObject->getName() << std::endl;
		}
	}

	// scan for activatables
	for (auto& obj : gameObject->transform.children)
	{
		if (obj->gameObject->getName().find("plate_") != std::string::npos)
		{
			std::cout << "Found plate: " << obj->gameObject->getName() << std::endl;
		}
	}
}