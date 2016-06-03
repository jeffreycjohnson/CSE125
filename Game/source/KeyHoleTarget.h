#pragma once
#include "Target.h"
#include "Sound.h"
#include "Config.h"

class KeyHoleTarget :
	public Target
{
private:
	Sound* keyinsert;
public:
	int keyHoleID;
	bool keyUsed;

	KeyHoleTarget();
	KeyHoleTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName);

	~KeyHoleTarget();

	void fixedUpdate() override;
	void create() override;
};

