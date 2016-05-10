#pragma once
#include "Target.h"

class KeyHoleTarget :
	public Target
{
public:
	KeyHoleTarget();
	KeyHoleTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);

	~KeyHoleTarget();

	void fixedUpdate() override;
};

