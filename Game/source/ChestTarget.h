#pragma once
#include "Target.h"

class ChestTarget :
	public Target
{
public:
	float openness = 0.0f;
	glm::vec3 initPosition;
	bool isOpen;

	ChestTarget();
	ChestTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	~ChestTarget();

	void create() override;
	void fixedUpdate() override;
};

