#pragma once
#include "Activator.h"
class ChestActivator :
	public Activator
{
public:
	bool isActivated;
	bool isNotActivated;
	ChestActivator();
	ChestActivator(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName);
	~ChestActivator();

	void trigger(bool activated);

	void trigger();

};

