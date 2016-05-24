#pragma once
#include "Activator.h"

class KeyActivator :
	public Activator
{
private:
public:
	int keyHoleID;

	KeyActivator();
	KeyActivator(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName);

	~KeyActivator();

	void trigger();

};

