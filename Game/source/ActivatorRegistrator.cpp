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
#include "KeyActivator.h"
#include "KeyHoleActivator.h"
#include "KeyHoleTarget.h"
#include "ChestActivator.h"
#include "ChestTarget.h"

std::map<std::string, targFun> ActivatorRegistrator::prefixToTarget =
{
	{ "rotate_" , [](auto args, auto idToTarget) {return new Rotating(args, idToTarget);}},
	{ "laser_",   [](auto args, auto idToTarget) {return new Laser(args, idToTarget); } },
	{ "vddoor_",  [](auto args, auto idToTarget) {return new Door(args, idToTarget, DOWN); } },
	{ "keyhole_",  [](auto args, auto idToTarget) {return new KeyHoleTarget(args, idToTarget); } },
	{ "chest_",  [](auto args, auto idToTarget) {return new ChestTarget(args, idToTarget); } },
	{ "key_",  [](auto args, auto idToTarget) {return new KeyTarget(args, idToTarget); } },
};

std::map<std::string, actvFun> ActivatorRegistrator::prefixToActivator =
{
	{ "plate_" ,  [](auto args, auto idToTarget) {return new Plate(args, idToTarget); } },
	{ "timebutton_" ,  [](auto args, auto idToTarget) {return new PressButton(args, idToTarget); } },
	{ "keyhole_" ,  [](auto args, auto idToTarget) {return new KeyHoleActivator(args, idToTarget); } },
	{ "key_" ,  [](auto args, auto idToTarget) {return new KeyActivator(args, idToTarget); } },
	{ "chest_" ,  [](auto args, auto idToTarget) {return new ChestActivator(args, idToTarget); } },
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
	std::map<int, Target*> idToTargets;

	// REGISTER TARGETS
	for (auto keyval : prefixToTarget) 
	{
		auto targs = GameObject::FindAllByPrefix(keyval.first);
		for (auto targ : targs) 
		{
			auto tokens = split(targ->getName(), '_');

			//MAGIC LAMBA. ASK ME ELTON FOR THE DETAILS
			Target *t = keyval.second(tokens, &idToTargets);
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
			Activator *activator = keyval.second(tokens, idToTargets);

			act->addComponent(activator);
			//Sound *sound = new Sound("zeldasecret", false, false, 1.0f, true);
			//act->addComponent(sound);
		}
	}
}