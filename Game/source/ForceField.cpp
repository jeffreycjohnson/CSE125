#include "ForceField.h"



ForceField::ForceField()
{
}


ForceField::~ForceField()
{
}

ForceField::ForceField(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName, bool turnsBackOn)
{
	canTurnBackOn = turnsBackOn;

	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[groupName + std::to_string(targetID)] = this;
}

void ForceField::fixedUpdate()
{
	if (isActivated() && gameObject->getVisible())
	{
		gameObject->setVisible(false);
		gameObject->setActive(false);
	}
	else if (canTurnBackOn && !isActivated() && !gameObject->getVisible()) {
		gameObject->setVisible(true);
		gameObject->setActive(true);
	}
}
