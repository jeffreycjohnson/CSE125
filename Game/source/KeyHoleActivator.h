#pragma once
#include "Activator.h"
class KeyHoleActivator :
	public Activator
{
public:
	KeyHoleActivator();
	KeyHoleActivator(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName);

	~KeyHoleActivator();
	void trigger();
};

