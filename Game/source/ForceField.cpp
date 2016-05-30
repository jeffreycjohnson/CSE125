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
				turnOn.push_back(Sound::affixSoundToDummy(collider->gameObject, new Sound("ff_on", false, false, file.getFloat("ff_on", "volume"), true)));
				turnOff.push_back(Sound::affixSoundToDummy(collider->gameObject, new Sound("ff_off", false, false, file.getFloat("ff_off", "volume"), true)));
				passiveHum.push_back(Sound::affixSoundToDummy(collider->gameObject, new Sound("ff_passive", true, true, file.getFloat("ff_passive", "volume"), true)));
			}
		}
	}
}

void ForceField::fixedUpdate()
{
	if (isActivated() && gameObject->getVisible())
	{
		gameObject->setVisible(false);
		gameObject->findChildByNameContains("StaticColliders")->setActive(false, SetAllChildren);
		for (auto hum : passiveHum) {
			hum->pause();
		}
		for (auto off : turnOff) {
			off->play();
		}
	}
	else if (!isActivated() && canTurnBackOn && !gameObject->getVisible()) {
		gameObject->setVisible(true);
		gameObject->findChildByNameContains("StaticColliders")->setActive(true, SetAllChildren);
		gameObject->setVisible(true);
		for (auto hum : passiveHum) {
			hum->pause();
		}
		for (auto on : turnOn) {
			on->play();
		}
	}
	else {
		bool anyPlaying = false;
		for (auto on : turnOn) {
			for (auto off : turnOff) {
				anyPlaying = anyPlaying || off->isPlaying() || on->isPlaying();
			}
		}

		// If no OFF or ON sounds are playing
		if (!anyPlaying) {
			for (auto hum : passiveHum) {
				hum->play();
			}
		}
	}
}
