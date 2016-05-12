#include "KeyHoleTarget.h"
#include "KeyHoleActivator.h"
#include <iostream>


KeyHoleTarget::KeyHoleTarget()
{
}

KeyHoleTarget::KeyHoleTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	keyHoleID = std::stoi(tokens[4]);
	int threshold = std::stoi(tokens[5]);

	setThreshold(threshold);
	(*idToTarget)[keyHoleID] = this;
}


KeyHoleTarget::~KeyHoleTarget()
{
}

void KeyHoleTarget::fixedUpdate()
{
	if (isActivated()) {
		// Do whatever Keyhole model changes and then 
		// call KeyHoleActivator->activate() to open door/chest
		this->gameObject->getComponent<KeyHoleActivator>()->trigger();
	}
}
