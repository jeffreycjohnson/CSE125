#include "ForceField.h"



ForceField::ForceField()
{
}


ForceField::~ForceField()
{
}

ForceField::ForceField(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;
}

void ForceField::fixedUpdate()
{
	if (isActivated() && gameObject->getVisible())
	{
		gameObject->setVisible(false);
		gameObject->setActive(false);
	}
}
