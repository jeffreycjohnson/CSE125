#pragma once
#include "Activator.h"

class Key :
	public Activator
{
private:
	bool isActivated;
public:
	int keyHoleID;

	Key();
	Key(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets);

	~Key();

	void trigger();

};

