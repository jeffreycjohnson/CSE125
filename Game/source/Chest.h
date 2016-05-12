#pragma once
#include "Target.h"

class Chest :
	public Target
{
public:
	float openness = 0.0f;
	glm::vec3 initPosition;

	Chest();
	Chest(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	~Chest();

	void create() override;
	void fixedUpdate() override;
};

