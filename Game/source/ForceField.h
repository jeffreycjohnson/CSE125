#pragma once
#include "Target.h"
#include "Sound.h"

class ForceField :
	public Target
{
public:
	bool canTurnBackOn;
	std::vector<Sound*> turnOn, turnOff, passiveHum;

	ForceField();
	~ForceField();

	ForceField(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName, bool turnsBackOn);
	void create() override;
	void fixedUpdate() override;
};

