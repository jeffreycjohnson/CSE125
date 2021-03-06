#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include "Component.h"
#include "OctreeManager.h"
#include "Sound.h"

#include "NetworkManager.h"
#include "NetworkStruct.h"

struct Sensitivity
{
	float mouseSensitivity;
	float joystickSensitivity;

	Sensitivity(float mouseSensitivity, float joystickSensitivity)
		: mouseSensitivity(mouseSensitivity), joystickSensitivity(joystickSensitivity)
	{
	}

	~Sensitivity()
	{
	}
};

class FPSMovement : public Component
{
private:
	// These have been moved to FPSMovement::loadGameSettings()
	static float baseHSpeed;
	static float baseVSpeed;
	static float startJumpSpeed;
	static float vAccel;
	static float deathFloor;
	static float interactDistance;

	// DeathSplash(tm)
	float deathTimer, deathDefaultTime;
	bool  deaded, justDeaded;

	OctreeManager* oct;
	BoxCollider* playerBoxCollider, *feetCollider;
	Collider* floor;
	GameObject* verticality;

	// NOTE: We don't actually want broadcasts to start from FPSMovement since there's 4 of them...
	Sound* jumpSound;
	Sound* landSound;
	Sound* testBroadcastSound;
	Sound* deathRattle;
	Sound* emoteSound;

	float mouseSensitivity, joystickSensitivity;

	glm::vec3 position, forward, volatile front, up, right, worldUp, moveDir;
	GLfloat yaw, pitch;
	RayHitInfo downHit;
	float playerRadius, playerHeightRadius, vSpeed, footRadius;
	bool hitWall, pastFirstTick, standingOnSurface, justJumped;
	int clientID;

	bool justClicked;

	glm::vec2 lastMousePosition;
	glm::vec3 initialPosition;

	// ray cast debugging
	glm::vec3 lastRayPoint;
	glm::vec3 lastRayPointPlusN;
	bool raycastHit;
	
	void robotEmote();
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
		int clientID, Sensitivity sensitivites,
		glm::vec3 initPosition, glm::vec3 upVector,
		GameObject* verticality = nullptr);
	~FPSMovement() {};

	static void loadGameSettings(ConfigFile&);

	void create() override;
	void fixedUpdate() override;
	void debugDraw() override;
	void respawn();
};

#endif // PLAYER_MOVEMENT_H