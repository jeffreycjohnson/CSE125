#include "ForceField.h"
#include "Config.h"


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

void ForceField::create()
{
	auto colNode = gameObject->findChildByNameContains("Colliders");
	ConfigFile file("config/sounds.ini");
	if (colNode) {
		for (auto collider : colNode->transform.children) {
			if (collider->gameObject) {
				passiveHum.push_back(Sound::affixSoundToDummy(collider->gameObject, new Sound("ff_passive", true, true, file.getFloat("ff_passive", "volume"), true)));
			}
		}
	}
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
		gameObject->findChildByNameContains("Colliders")->setActive(false, SetAllChildren);
		for (auto hum : passiveHum) {
			hum->pause();
		}
	}
	else if (!isActivated() && canTurnBackOn && !gameObject->getVisible()) {
		gameObject->setVisible(true);
		gameObject->findChildByNameContains("Colliders")->setActive(true, SetAllChildren);
		gameObject->setVisible(true);
		for (auto hum : passiveHum) {
			hum->pause();
		}
	}

}
