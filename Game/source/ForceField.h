#pragma once
#include "Target.h"

class ForceField :
	public Target
{
public:
	
	ForceField();
	~ForceField();

	ForceField(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName);
	void fixedUpdate() override;
};

