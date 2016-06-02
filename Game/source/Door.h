#ifndef DOOR_H
#define DOOR_H

#include "Target.h"
#include "Sound.h"

#include <map>
#include <vector>

enum DoorMovement
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	BOTH
};

class Door : public Target
{
private:
	DoorMovement moveDirection;
	Sound* unlockSound;
	glm::vec3 initPosit;
	bool reopen, locked;
	float openness = -0.1f;

	glm::vec3 moveDirectionVec();
public:
	Door();
	Door(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, DoorMovement moveDirection, std::string groupName, bool canReopen);
	
	~Door();

	void create() override;
	void fixedUpdate() override;
};

#endif // DOOR_H