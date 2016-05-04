#include "ActivatorRegistrator.h"

#include <iostream>
#include <sstream>
#include <functional>

#include "GameObject.h"

#include "Laser.h"
#include "Rotating.h"
#include "Plate.h"


//http://blog.noctua-software.com/object-factory-c++.html

std::map<std::string, std::function<Target*(std::vector<std::string>)>> ActivatorRegistrator::prefixToTarget =
{
	{ "rotate_" , [](std::vector<std::string> args) {return new Rotating();}},
	{ "laser_",   [](std::vector<std::string> args) {return new FixedLaser();}},
};

std::map<std::string, std::function<Activator*(std::vector<std::string>)>> ActivatorRegistrator::prefixToActivator =
{
	{ "plate_" ,  [](std::vector<std::string> args) {return new Plate();}},
};



std::vector<std::string> split(const std::string &s, char delim) {
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

	/// TODO OBVIOUSLY HANDLE MORE TARGETS
	std::map<int, Target*> idToTargets;

	// REGISTER TARGETS
	for (auto keyval : prefixToTarget) {
		auto targs = GameObject::FindAllByPrefix(keyval.first);
		for (auto targ : targs) {
			auto tokens = split(targ->getName(), '_');

			//MAGIC LAMBA. ASK ME ELTON FOR THE DETAILS
			Target *t = keyval.second(tokens);

			//TODO Placeholder
			int targetID = std::stoi(tokens[1]);
			int threshold = std::stoi(tokens[2]);

			t->setThreshold(threshold);

			targ->addComponent(t);
			idToTargets[targetID] = t;
		}
	}

	// REGISTER ACTIVATORS
	for (auto keyval : prefixToActivator) {
		auto activators = GameObject::FindAllByPrefix(keyval.first);
		for (auto act : activators) {
			auto tokens = split(act->getName(), '_');

			Activator * activator = keyval.second(tokens);

			//PlaceHolder code
			for (int i = 1; i < tokens.size(); i += 3) {
				int targetID = std::stoi(tokens[i + 0]);
				TriggerType triggerType = strToTriggerType(tokens[i + 1]);
				int activatorID = std::stoi(tokens[i + 2]);

				activator->addConnection(Connection(idToTargets.at(targetID), triggerType));
			}

			//Add Component to current Game object
			act->addComponent(activator);
		}
	}
}