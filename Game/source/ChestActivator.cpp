#include "ChestActivator.h"
#include <iostream>


ChestActivator::ChestActivator()
{
}

ChestActivator::ChestActivator(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets)
{

	int targetID = std::stoi(tokens[1]);
	TriggerType triggerType = strToTriggerType(tokens[2]);
	int activatorID = std::stoi(tokens[3]); // not really being used. just to keep uniqueness of names in blender.

	this->addConnection(Connection(idToTargets.at(targetID), triggerType));
}

ChestActivator::~ChestActivator()
{
}

void ChestActivator::trigger(bool activated)
{
	// we're on the edge!!
	if (!activated && !isNotActivated)
	{
		isNotActivated = true;
		deactivate();
	}

	// we're on the edge!!
	if (activated && isNotActivated)
	{
		isNotActivated = false;
		activate();
		std::cout << "chest is activating key" << std::endl;
	}

	
}

void ChestActivator::trigger() {
	activate();
}

