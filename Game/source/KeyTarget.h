#pragma once
#include "Target.h"

class KeyTarget :
	public Target
{
private:
public:
	glm::vec3 initPosition;
	float openness = 0.0f;
	bool canBePickedUp; // allows keys not contained in chests if proper blender label
	bool pickedUp; // stops movement animation when chest opened

	KeyTarget();
	KeyTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	~KeyTarget();

	void fixedUpdate() override;
	void create() override;
};

