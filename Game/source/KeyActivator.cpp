#include "KeyActivator.h"



KeyActivator::KeyActivator()
{
}

KeyActivator::KeyActivator(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets)
{
	keyHoleID = std::stoi(tokens[1]);
	TriggerType triggerType = strToTriggerType(tokens[2]);
	int activatorID = std::stoi(tokens[3]); // not really being used. just to keep uniqueness of names in blender.

	this->addConnection(Connection(idToTargets.at(keyHoleID), triggerType));
	
}

KeyActivator::~KeyActivator()
{
}

void KeyActivator::trigger()
{
	// KeyActivator has been brought to KeyHole so activate KeyHoleTarget
	activate();
}
