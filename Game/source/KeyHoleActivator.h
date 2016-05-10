#pragma once
#include "Activator.h"
class KeyHoleActivator :
	public Activator
{
public:
	KeyHoleActivator();
	KeyHoleActivator(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets);

	~KeyHoleActivator();
	void trigger();
};

