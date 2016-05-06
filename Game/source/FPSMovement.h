#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"

class FPSMovement : public Component
{
private:
	const float baseVSpeed = -0.2f;
	const float startJumpSpeed = 0.3f;
	const float vAccel = -0.02f;
	const float speed = 1.0f;

	GameObject* verticality;

	float moveSpeed, mouseSensitivity;

	glm::vec3 position, front, up, right, worldUp, moveDir;
	GLfloat yaw, pitch;
	float playerRadius, playerHeightRadius, vSpeed;
	bool hitWall, pastFirstTick;
	int clientID;

	glm::vec2 lastMousePosition;
	glm::vec3 initialPosition;

	// ray cast debugging
	glm::vec3 lastRayPoint;
	bool raycastHit;

	void recalculate();
	glm::vec3 handleRayCollision(glm::vec3 position, glm::vec3 castDirection, glm::vec3 moveDirection);
public:
	FPSMovement(
		int clientID, float moveSpeed, float mouseSensitivity,
		glm::vec3 position, glm::vec3 up,
		GameObject* verticality = nullptr);
	~FPSMovement() {};

	void create() override;
	void fixedUpdate() override;
	void debugDraw() override;

	void respawn();
};

#endif // PLAYER_MOVEMENT_H

