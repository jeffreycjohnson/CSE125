#ifndef DOOR_H
#define DOOR_H

#include "Target.h"

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
	Door(int activationThreshold, DoorMovement moveDirection);
	~Door();

	void create() override;
	void fixedUpdate() override;
};

#endif // DOOR_H