#pragma once
#include "Target.h"

class KeyTarget :
	public Target
{
private:
public:
	glm::vec3 initPosition;
	float openness = -0.1f;
	bool canBePickedUp; // allows keys not contained in chests if proper blender label
	bool pickedUp; // stops movement animation when chest opened

	KeyTarget();
	KeyTarget(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName);
	~KeyTarget();

	void fixedUpdate() override;
	void create() override;
};

