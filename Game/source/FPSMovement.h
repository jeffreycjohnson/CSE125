#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"

class FPSMovement : public Component
{
private:
	float moveSpeed, mouseSensitivity;

	glm::vec3 position, front, up, right, worldUp;
	GLfloat yaw, pitch;

	glm::vec2 lastMousePosition;

	void recalculate();
public:
	FPSMovement(
		float moveSpeed, float mouseSensitivity,
		glm::vec3 position, glm::vec3 up);
	~FPSMovement() {};

	void create() override;
	void update(float dt) override;
};

#endif // PLAYER_MOVEMENT_H

