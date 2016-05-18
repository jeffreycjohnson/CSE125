#pragma once
#include "Target.h"

class ForceField :
	public Target
{
public:
	
	ForceField();
	~ForceField();

	ForceField(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	void fixedUpdate() override;
};

