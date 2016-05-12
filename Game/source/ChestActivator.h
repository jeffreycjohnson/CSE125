#pragma once
#include "Activator.h"
class ChestActivator :
	public Activator
{
public:
	ChestActivator();
	ChestActivator(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets);
	~ChestActivator();

	void trigger();

};

