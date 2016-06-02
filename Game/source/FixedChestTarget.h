#pragma once
#include "Target.h"

class FixedChestTarget :
	public Target
{
public:
	float openness = -0.1f;
	glm::vec3 initPosition;
	bool isOpened;

	FixedChestTarget();
	FixedChestTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName);
	~FixedChestTarget();

	void create() override;
	void fixedUpdate() override;
};

