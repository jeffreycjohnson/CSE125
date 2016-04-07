#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"

class PlayerMovement : public Component
{
public:
	PlayerMovement() {};
	~PlayerMovement() {};

	void update(float dt) override;
};

#endif // PLAYER_MOVEMENT_H

