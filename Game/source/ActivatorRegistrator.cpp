#include "ActivatorRegistrator.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "GameObject.h"

#include "Rotating.h"
#include "Plate.h"

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

	/// TODO OBVIOUSLY HANDLE MORE ACTIVATABLES

	std::map<int, Target*> idToTargets;
	auto rotates = GameObject::FindAllByPrefix("rotate_");
	for (auto& rotate : rotates)
	{
		/// TODO ADD EXTRA PARAMATER TO CONTROL ACTIVATION THRESHOLD
		auto tokens = split(rotate->getName(), '_');
		int targetID = std::stoi(tokens[1]);
		int threshold = std::stoi(tokens[2]);

		Target *t = new Rotating(threshold);
		rotate->addComponent(t);

		idToTargets[targetID] = t;
	}

	auto plates = GameObject::FindAllByPrefix("plate_");
	for (auto& plate : plates)
	{
		auto tokens = split(plate->getName(), '_');
		int targetID = std::stoi(tokens[1]);
		TriggerType triggerType = strToTriggerType(tokens[2]);

		Activator* activator = new Plate;
		activator->addConnection(Connection(idToTargets.at(targetID), triggerType));

		/// TODO THIS ONLY NECESSARY DUE TO LACK OF BUBBLING
		plate->transform.children[0]->children[0]->gameObject->addComponent(activator);
	}

	int x = 5;
}