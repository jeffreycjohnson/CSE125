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

void ForceField::create()
{
	turnOn = Sound::affixSoundToDummy(gameObject, new Sound("ff_on", false, false, 0.25f, true));
	turnOff = Sound::affixSoundToDummy(gameObject, new Sound("ff_off", false, false, 0.25f, true));
	passiveHum = Sound::affixSoundToDummy(gameObject, new Sound("ff_passive", true, true, 0.25f, true));
}

void ForceField::fixedUpdate()
{
	if (isActivated() && gameObject->getVisible())
	{
		gameObject->setVisible(false);
		gameObject->findChildByNameContains("StaticColliders")->setActive(false, SetAllChildren);
		passiveHum->pause();
		turnOff->play();
	}
	else if (!isActivated() && canTurnBackOn && !gameObject->getVisible()) {
		gameObject->setVisible(true);
		gameObject->findChildByNameContains("StaticColliders")->setActive(true, SetAllChildren);
		gameObject->setVisible(true);
		passiveHum->pause();
		turnOn->play();
	}
	else {
		if (!turnOn->isPlaying() && !turnOff->isPlaying()) {
			passiveHum->play();
		}
	}
}
