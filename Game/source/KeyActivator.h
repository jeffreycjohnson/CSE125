#pragma once
#include "Activator.h"

class KeyActivator :
	public Activator
{
private:
public:
	int keyHoleID;

	KeyActivator();
	KeyActivator(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets);

	~KeyActivator();

	void trigger();

};

