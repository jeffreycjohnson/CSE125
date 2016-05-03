#ifndef ACTIVATOR_H
#define ACTIVATOR_H

#include "Component.h"
#include "GameObject.h"

#include "Target.h"

#include <vector>
#include <stdexcept>
#include <string>
#include <map>

//http://blog.noctua-software.com/object-factory-c++.html

class ActivatorFactory
{
public:
	virtual Activator *create() = 0;
};

class Target;

enum TriggerType
{
	POSITIVE,
	NEGATIVE,
	ZERO
};

TriggerType strToTriggerType(const std::string& str);

struct Connection
{
	TriggerType triggerType;
	Target* target;

	Connection() {}
	Connection(Target* target, TriggerType triggerType)
		: target(target), triggerType(triggerType) 
	{
	}
};

class Activator :
	public Component
{
private:
	TriggerType triggerType;
	std::vector<Connection> connections;
	static std::map<std::string, ActivatorFactory*> factories;

public:
	Activator();
	virtual ~Activator() = 0;

	void addConnection(Connection connection);

	static void registerType(
		const std::string& name, ActivatorFactory *factory)
	{
		factories[name] = factory;
	}
	void activate();
	void deactivate();
};

#endif ACTIVATOR_H

