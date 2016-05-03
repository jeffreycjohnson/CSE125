#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"

class FPSMovement : public Component
{
private:
	GameObject* verticality;

	float moveSpeed, mouseSensitivity;

	glm::vec3 position, front, up, right, worldUp, moveDir;
	GLfloat yaw, pitch;

	float playerRadius = 1;

	glm::vec2 lastMousePosition;

	// ray cast debugging
	glm::vec3 lastRayPoint;
	bool raycastHit;

	void recalculate();
public:
	FPSMovement(
		float moveSpeed, float mouseSensitivity,
		glm::vec3 position, glm::vec3 up,
		GameObject* verticality = nullptr);
	~FPSMovement() {};

	void create() override;
	void fixedUpdate() override;
	void update(float dt) override;
	void debugDraw() override;

};

#endif // PLAYER_MOVEMENT_H

