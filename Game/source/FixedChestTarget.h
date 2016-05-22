#pragma once
#include "Target.h"

class FixedChestTarget :
	public Target
{
public:
	float openness = 0.0f;
	glm::vec3 initPosition;
	bool isOpened;

	FixedChestTarget();
	FixedChestTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	~FixedChestTarget();

	void create() override;
	void fixedUpdate() override;
};

