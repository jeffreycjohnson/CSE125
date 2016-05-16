#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"
#include "OctreeManager.h"

class FPSMovement : public Component
{
private:
	const float baseVSpeed = -0.2f;
	const float startJumpSpeed = 0.25f;
	const float vAccel = -0.015f;
	const float deathFloor = -20.0f;

	OctreeManager* oct;
	BoxCollider* playerBoxCollider;
	GameObject* verticality;

	float moveSpeed, mouseSensitivity;

	glm::vec3 position, forward, front, up, right, worldUp, moveDir;
	GLfloat yaw, pitch;
	RayHitInfo downHit;
	float playerRadius, playerHeightRadius, vSpeed;
	bool hitWall, pastFirstTick, standingOnSurface, justJumped;
	int clientID;

	glm::vec2 lastMousePosition;
	glm::vec3 initialPosition;

	// ray cast debugging
	glm::vec3 lastRayPoint;
	glm::vec3 lastRayPointPlusN;
	bool raycastHit;

	void handleHorizontalMovement(float dt);
	void handleVerticalMovement(float dt);
	void checkOnSurface(glm::vec3 position, glm::vec3 direction);
	void getPlayerRadii();
	void recalculate();
	bool slideAgainstWall(glm::vec3 position, glm::vec3 castDirection, int failCount);
	void pushOutOfAdjacentWalls(glm::vec3 position, glm::vec3 direction);
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