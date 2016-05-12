#pragma once
#include "Target.h"

class KeyTarget :
	public Target
{
private:
public:
	KeyTarget();
	KeyTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	~KeyTarget();
	bool canBePickedUp; // allows keys not contained in chests if proper blender label
};

