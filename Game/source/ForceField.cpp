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

	if (tokens.size() > 3 && tokens[3] == "f")
	{ 
		fixedThreshold = std::stoi(tokens[4]);
	}

	setThreshold(threshold);
	(*idToTarget)[groupName + std::to_string(targetID)] = this;
}

void ForceField::fixedUpdate()
{
	if (activationLevel() >= fixedThreshold && !isZeroedOut())
	{
		canTurnBackOn = false;
	}

	if (isActivated() && gameObject->getVisible())
	{
		gameObject->setVisible(false);
		gameObject->findChildByNameContains("StaticColliders")->setActive(false, SetAllChildren);
	}
	else if (!isActivated() && canTurnBackOn && !gameObject->getVisible()) {
		gameObject->setVisible(true);
		gameObject->findChildByNameContains("StaticColliders")->setActive(true, SetAllChildren);
		gameObject->setVisible(true);

	}
}
