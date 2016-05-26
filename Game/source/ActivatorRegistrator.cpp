#include "ActivatorRegistrator.h"

#include <iostream>
#include <sstream>
#include <functional>

#include "GameObject.h"
#include "Sound.h"

#include "Laser.h"
#include "Rotating.h"
#include "Plate.h"
#include "PressButton.h"
#include "Door.h"
#include "KeyTarget.h"
#include "FixedKeyTarget.h"
#include "KeyActivator.h"
#include "KeyHoleActivator.h"
#include "KeyHoleTarget.h"
#include "ChestActivator.h"
#include "FixedChestTarget.h"
#include "ChestTarget.h"
#include "ForceField.h"

std::map<std::string, targFun> ActivatorRegistrator::prefixToTarget =
{
	{ "rotate_" , [](auto args, auto idToTarget, auto groupName) {return new Rotating(args, idToTarget, groupName);}},
	{ "laser_",   [](auto args, auto idToTarget, auto groupName) {return new Laser(args, idToTarget, false, groupName); } },
	{ "fixedlaser_",   [](auto args, auto idToTarget, auto groupName) {return new Laser(args, idToTarget, true, groupName); } },
	{ "forcefield_",   [](auto args, auto idToTarget, auto groupName) {return new ForceField(args, idToTarget, groupName, false); } },
	{ "tforcefield_",   [](auto args, auto idToTarget, auto groupName) {return new ForceField(args, idToTarget, groupName, true); } },
	//{ "vddoor_",  [](auto args, auto idToTarget, auto groupName) {return new Door(args, idToTarget, DOWN, groupName); } },
	{ "splitdoor_",  [](auto args, auto idToTarget, auto groupName) {return new Door(args, idToTarget, BOTH, groupName); } },
	{ "keyhole_",  [](auto args, auto idToTarget, auto groupName) {return new KeyHoleTarget(args, idToTarget, groupName); } },
	{ "fixedchest_",  [](auto args, auto idToTarget, auto groupName) {return new FixedChestTarget(args, idToTarget, groupName); } },
	{ "chest",  [](auto args, auto idToTarget, auto groupName) {return new ChestTarget(args, idToTarget, groupName); } },
	{ "key_",  [](auto args, auto idToTarget, auto groupName) {return new KeyTarget(args, idToTarget, groupName); } },
	{ "fixedkey_",  [](auto args, auto idToTarget, auto groupName) {return new FixedKeyTarget(args, idToTarget, groupName); } },
};

std::map<std::string, actvFun> ActivatorRegistrator::prefixToActivator =
{
	{ "plate_" ,  [](auto args, auto idToTarget, auto groupName) {return new Plate(args, idToTarget, groupName); } },
	{ "pressbutton_" ,  [](auto args, auto idToTarget, auto groupName) {return new PressButton(args, idToTarget, groupName); } },
	{ "keyhole_" ,  [](auto args, auto idToTarget, auto groupName) {return new KeyHoleActivator(args, idToTarget, groupName); } },
	{ "key_" ,  [](auto args, auto idToTarget, auto groupName) {return new KeyActivator(args, idToTarget, groupName); } },
	{ "fixedkey_" ,  [](auto args, auto idToTarget, auto groupName) {return new KeyActivator(args, idToTarget, groupName); } },
	{ "fixedchest_" ,  [](auto args, auto idToTarget, auto groupName) {return new ChestActivator(args, idToTarget, groupName); } },
	{ "chest_" ,  [](auto args, auto idToTarget, auto groupName) {return new ChestActivator(args, idToTarget, groupName); } },
};

std::vector<std::string> split(const std::string &s, char delim) 
{
	std::vector<std::string> elems;

	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) 
	{
		if (!item.empty()) elems.push_back(item);
	}

	return elems;
}

ActivatorRegistrator::ActivatorRegistrator()
{
}


ActivatorRegistrator::~ActivatorRegistrator()
{
}

void ActivatorRegistrator::create()
{
	std::cout << "Registering activators..." << std::endl;
	std::map<std::string, Target*> idToTargets;

	// REGISTER TARGETS
	for (auto keyval : prefixToTarget) 
	{
		auto targs = GameObject::FindAllByPrefix(keyval.first);
		for (auto targ : targs) 
		{
			auto tokens = split(targ->getName(), '_');
			
			Transform * parent = targ->transform.getParent();

			while (parent && parent->getParent() && parent->getParent()->getParent() && parent->getParent()->getParent()->getParent()) {
				parent = parent->getParent();
			}
			std::string parentName = parent ? parent->gameObject->getName() : "";

			//MAGIC LAMBA. ASK ME ELTON FOR THE DETAILS
			Target *t = keyval.second(tokens, &idToTargets, parentName);
			targ->addComponent(t);

			//Sound Testing
			//targ->addComponent();
		}
	}

	// REGISTER ACTIVATORS
	for (auto keyval : prefixToActivator) 
	{
		auto activators = GameObject::FindAllByPrefix(keyval.first);
		for (auto act : activators) 
		{
			auto tokens = split(act->getName(), '_');

			Transform * parent = act->transform.getParent();

			while (parent && parent->getParent() && parent->getParent()->getParent() && parent->getParent()->getParent()->getParent()) {
				parent = parent->getParent();
			}
			std::string parentName = parent ? parent->gameObject->getName() : "";

			Activator *activator = keyval.second(tokens, idToTargets, parentName);

			act->addComponent(activator);
			//Sound *sound = new Sound("zeldasecret", false, false, 1.0f, true);
			//act->addComponent(sound);
		}
	}
}
