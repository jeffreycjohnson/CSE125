#include "PlayerMovement.h"

#include "GameObject.h"
#include "Input.h"

const float SPEED = 3.0f;

void PlayerMovement::update(float dt)
{
	gameObject->transform.translate(Input::getAxis("roll") * SPEED * dt, Input::getAxis("yaw") * SPEED * dt, Input::getAxis("pitch") * SPEED * dt);
}