#pragma once
#include "Target.h"

class KeyHoleTarget :
	public Target
{
public:
	int keyHoleID;
	bool keyUsed;

	KeyHoleTarget();
	KeyHoleTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName);

	~KeyHoleTarget();

	void fixedUpdate() override;
};

