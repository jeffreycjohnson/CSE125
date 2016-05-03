#include "ActivatorRegistrator.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "GameObject.h"

#include "Laser.h"
#include "Rotating.h"
#include "Plate.h"


//http://blog.noctua-software.com/object-factory-c++.html
class TargetFactory
{
public:
	virtual Target *create() = 0;
};

class ActivatorFactory
{
public:
	virtual Activator *create() = 0;
};


#define REGISTER_TARGET(klass) \
    class klass##Factory : public TargetFactory { \
    public: \
        klass##Factory() \
        { \
            Object::registerType(#klass, this); \
        } \
        virtual Object *create() { \
            return new klass(); \
        } \
    }; \
    static klass##Factory global_##klass##Factory;

#define REGISTER_ACTIVATOR(klass) \
    class klass##Factory : public ActivatorFactory { \
    public: \
        klass##Factory() \
        { \
            Object::registerType(#klass, this); \
        } \
        virtual Object *create() { \
            return new klass(); \
        } \
    }; \
    static klass##Factory global_##klass##Factory;


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

	auto lasers = GameObject::FindAllByPrefix("laser_");
	for (auto& laser : lasers)
	{
		/// TODO ADD EXTRA PARAMATER TO CONTROL ACTIVATION THRESHOLD
		auto tokens = split(laser->getName(), '_');
		int targetID = std::stoi(tokens[1]);
		int threshold = std::stoi(tokens[2]);

		Target *t = new FixedLaser(threshold);
		laser->addComponent(t);

		idToTargets[targetID] = t;
	}

	// REGISTER ACTIVATORS
	auto plates = GameObject::FindAllByPrefix("plate_");
	for (auto& plate : plates)
	{
		auto tokens = split(plate->getName(), '_');

		Activator* activator = new Plate;
		for (int i = 1; i < tokens.size(); i += 3)
		{
			int targetID = std::stoi(tokens[i + 0]);
			TriggerType triggerType = strToTriggerType(tokens[i + 1]);
			int activatorID = std::stoi(tokens[i + 2]);

			activator->addConnection(Connection(idToTargets.at(targetID), triggerType));
		}

		/// TODO THIS ONLY NECESSARY DUE TO LACK OF BUBBLING
		plate->addComponent(activator);
	}

	int x = 5;
}