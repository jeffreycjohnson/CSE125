#ifndef DOOR_H
#define DOOR_H

#include "Target.h"

#include <map>
#include <vector>

enum DoorMovement
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

class Door : public Target
{
private:
	DoorMovement moveDirection;
	glm::vec3 initPosit;

	float openness = 0.0;

	glm::vec3 moveDirectionVec();
public:
	Door();
	Door(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget, DoorMovement moveDirection);
	
	~Door();

	void create() override;
	void fixedUpdate() override;
};

#endif // DOOR_H