#include "KeyHoleTarget.h"
#include "KeyHoleActivator.h"
#include <iostream>
#include "Mesh.h"
#include "Material.h"

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


void KeyHoleTarget::loadKeyHoleMaterials()
{
	auto * r = new Material("assets/TerminalRedUnlocked.mat.ini", false);
	auto * o = new Material("assets/TerminalOrangeUnlocked.mat.ini", false);
	auto * y = new Material("assets/TerminalYellowUnlocked.mat.ini", false);
	auto * g = new Material("assets/TerminalGreenUnlocked.mat.ini", false);
	auto * b = new Material("assets/TerminalBlueUnlocked.mat.ini", false);
	auto * v = new Material("assets/TerminalVioletUnlocked.mat.ini", false);
}

KeyHoleTarget::~KeyHoleTarget()
{
}



void KeyHoleTarget::fixedUpdate()
{
	if (isActivated() && !keyUsed) {
		// Do whatever Keyhole model changes and then 
		// call KeyHoleActivator->activate() to open door/chest

		Mesh * mesh = this->gameObject->transform.children[1]->gameObject->getComponent<Mesh>();
		if (mesh != nullptr) {
			Material * m = mesh->getMaterial();
			std::string matName = m->getWatcherFileName();
			std::size_t pos = matName.find(".");
			std::string newMatName = matName.substr(0, pos);
			newMatName = newMatName + "Unlocked.mat.ini";
			mesh->setMaterial(new Material(newMatName, false));
		}
		this->gameObject->getComponent<KeyHoleActivator>()->trigger();
		keyUsed = true;
		keyinsert->play();
	}
}

void KeyHoleTarget::create() {
	ConfigFile file("config/sounds.ini");
	keyinsert = Sound::affixSoundToDummy(gameObject, new Sound("keychirp", true, false, file.getFloat("keychirp", "volume"), true));
}
