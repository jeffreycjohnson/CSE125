#include "KeyHoleTarget.h"
#include "KeyHoleActivator.h"
#include <iostream>


KeyHoleTarget::KeyHoleTarget()
{
}

KeyHoleTarget::KeyHoleTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName)
{
	keyHoleID = std::stoi(tokens[4]);
	int threshold = std::stoi(tokens[5]);

	setThreshold(threshold);
	(*idToTarget)[groupName + std::to_string(keyHoleID)] = this;
}


KeyHoleTarget::~KeyHoleTarget()
{
}

void KeyHoleTarget::fixedUpdate()
{
	if (isActivated() && !keyUsed) {
		// Do whatever Keyhole model changes and then 
		// call KeyHoleActivator->activate() to open door/chest
		this->gameObject->getComponent<KeyHoleActivator>()->trigger();
		keyUsed = true;
		keyinsert->play();
	}
}

void KeyHoleTarget::create() {
	ConfigFile file("config/sounds.ini");
	keyinsert = Sound::affixSoundToDummy(gameObject, new Sound("keychirp", true, false, file.getFloat("keychirp", "volume"), true));
}
