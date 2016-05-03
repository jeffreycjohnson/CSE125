#include "Activator.h"



Activator::Activator()
{
}


Activator::~Activator()
{
}

void Activator::addConnection(Connection connection)
{
	connections.push_back(connection);
}

void Activator::activate()
{
	for (auto& connection : connections)
	{
		connection.target->applyTrigger(connection.triggerType);
	}
}

void Activator::deactivate()
{
	for (auto& connection : connections)
	{
		connection.target->releaseTrigger(connection.triggerType);
	}
}

TriggerType strToTriggerType(const std::string & str)
{
	if (str == "p")
	{
		return POSITIVE;
	}
	else if (str == "n")
	{
		return NEGATIVE;
	}
	else if (str == "z")
	{
		return ZERO;
	}
	else
	{
		throw std::runtime_error("Tried parsing unknown trigger type");
	}
}
