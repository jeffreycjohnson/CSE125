#include "KeyHoleActivator.h"



KeyHoleActivator::KeyHoleActivator()
{
}

KeyHoleActivator::KeyHoleActivator(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets)
{

	int targetID = std::stoi(tokens[1]);
	TriggerType triggerType = strToTriggerType(tokens[2]);
	int activatorID = std::stoi(tokens[3]); // not really being used. just to keep uniqueness of names in blender.

	this->addConnection(Connection(idToTargets.at(targetID), triggerType));
}


KeyHoleActivator::~KeyHoleActivator()
{
}

void KeyHoleActivator::trigger()
{
	// keyhole has been unlocked so activate corresponding target. i.e. door, chest, etc.
	activate();
}
