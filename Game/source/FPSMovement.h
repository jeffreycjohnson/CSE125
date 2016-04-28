#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"

class FPSMovement : public Component
{
private:
	GameObject* verticality;

	int clientId;
	float moveSpeed, mouseSensitivity;

	glm::vec3 position, front, up, right, worldUp;
	GLfloat yaw, pitch;

	bool pastFirstTick;
	glm::vec2 lastMousePosition;

	void recalculate();
public:
	FPSMovement(
		int clientId, float moveSpeed, float mouseSensitivity,
		glm::vec3 position, glm::vec3 up,
		GameObject* verticality = nullptr);
	~FPSMovement() {};

	void create() override;
	void fixedUpdate() override;
};

#endif // PLAYER_MOVEMENT_H

