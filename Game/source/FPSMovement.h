#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"

class FPSMovement : public Component
{
private:
	const float baseVSpeed = -0.2f;
	const float startJumpSpeed = 12.5f;
	const float vAccel = -0.75f;
	const float deathFloor = -20.0f;

	GameObject* verticality;

	glm::vec3 position, front, up, right, worldUp, moveDir;
	GLfloat yaw, pitch;
	float moveSpeed, mouseSensitivity, playerRadius, playerHeightRadius, vSpeed;
	bool pastFirstTick, setVerticalityForward;
	int clientID;

	glm::vec2 lastMousePosition;
	glm::vec3 initialPosition;

	// ray cast debugging
	glm::vec3 lastRayPoint;
	bool raycastHit;

	void handleHorizontalMovement(float dt);
	void handleVerticalMovement(float dt);
	void getPlayerRadii();
	void recalculate();
	void handleWallSlide(glm::vec3 position, glm::vec3 castDirection);
	void raycastMouse();
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

