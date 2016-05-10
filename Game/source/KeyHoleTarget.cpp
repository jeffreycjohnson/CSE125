#include "KeyHoleTarget.h"
#include <iostream>


KeyHoleTarget::KeyHoleTarget()
{
}

KeyHoleTarget::KeyHoleTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;
}


KeyHoleTarget::~KeyHoleTarget()
{
}

void KeyHoleTarget::fixedUpdate()
{
	if (isActivated()) {
		// Do whatever Keyhole model changes and then 
		// call KeyHoleActivator->activate() to open door/chest
		std::cout << "KeyholeTarget is activated" << std::endl;
	}
}
