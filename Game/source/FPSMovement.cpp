#include "FPSMovement.h"

#include <iostream>

#include <glm/glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "GameObject.h"
#include "Input.h"
#include "Renderer.h"
#include "Timer.h"
#include "OctreeManager.h"
#include "Camera.h"
#include "Collider.h"
#include "Collision.h"
#include "BoxCollider.h"
#include "ServerInput.h"

const float SPEED = 3.0f;

FPSMovement::FPSMovement(int clientID, float moveSpeed, float mouseSensitivity, glm::vec3 position, glm::vec3 up, GameObject* verticality)
	: clientID(clientID), moveSpeed(moveSpeed), mouseSensitivity(mouseSensitivity), position(position), up(up), worldUp(up), verticality(verticality)
{
	this->front = glm::vec3(0, 0, -1);

	this->yaw = 0.0f;
	this->pitch = 0.0f;
	pastFirstTick = false;
	raycastHit = false;
}

void FPSMovement::create()
{
	if (verticality) this->gameObject->addChild(verticality);

	//Input::hideCursor();
	recalculate();
}

void FPSMovement::fixedUpdate()
{
	auto dt = Timer::fixedTimestep;

	if (!pastFirstTick)
	{
		pastFirstTick = true;

		glm::vec2 currMousePosition = ServerInput::mousePosition(clientID);
		lastMousePosition = currMousePosition;
		return;
	}

	RayHitInfo moveHit;

	auto oct = GameObject::SceneRoot.getComponent<OctreeManager>();
	if (oct != nullptr) {
		if (playerRadiusTime == 0) {
			Transform playerTrans = GameObject::FindByName("Player")->transform;
			GameObject * go = playerTrans.children[0]->children[0]->gameObject;
			BoxCollider * b = go->getComponent<BoxCollider>();
			playerRadiusTime = b->getWidth() / 2;
			std::cout << playerRadiusTime << std::endl;
		}

		hitWall = false;
		glm::vec3 newMoveVec = moveDir;

		newMoveVec = handleRayCollision(position, moveDir, newMoveVec);
		newMoveVec = handleRayCollision(position, glm::vec3(moveDir.z, moveDir.y, -moveDir.x), newMoveVec);
		newMoveVec = handleRayCollision(position, glm::vec3(-moveDir.z, moveDir.y, moveDir.x), newMoveVec);

		position += newMoveVec;
		gameObject->transform.setPosition(position.x, position.y, position.z);
	}

	glm::vec2 currMousePosition = ServerInput::mousePosition(clientID);
	glm::vec2 mouseDelta = currMousePosition - lastMousePosition;

	yaw += mouseDelta.x * mouseSensitivity;
	pitch += mouseDelta.y * mouseSensitivity;

	// can't look past certain angles
	pitch = fmaxf(-85.0f, fminf(85.0f, pitch));

	lastMousePosition = currMousePosition;

	// act on keyboard
	float speed = moveSpeed * dt;
	glm::vec3 worldFront = glm::normalize(glm::cross(worldUp, right));
	glm::vec3 normRight = glm::normalize(right);

	glm::vec3 xComp = ServerInput::getAxis("roll", clientID) * worldFront * speed;
	glm::vec3 zComp = ServerInput::getAxis("pitch", clientID) * normRight * speed;

	moveDir = xComp + zComp;
	if (!hitWall) {
		position += xComp + zComp;
	}

	recalculate();

}

glm::vec3 FPSMovement::handleRayCollision(glm::vec3 position, glm::vec3 castDirection, glm::vec3 moveDirection) {
	auto oct = GameObject::SceneRoot.getComponent<OctreeManager>();
	Ray moveRay(position, castDirection);
	RayHitInfo moveHit = oct->raycast(moveRay, Octree::STATIC_ONLY);
	glm::vec3 newMoveVec = moveDirection;
	if (moveHit.intersects && moveHit.hitTime <= playerRadiusTime && moveHit.hitTime >= 0) {
		if (moveHit.normal.x != 0)
			newMoveVec.x = 0;
		if (moveHit.normal.z != 0)
			newMoveVec.z = 0;

		hitWall = true;
	}
	return newMoveVec;
}

void FPSMovement::debugDraw()
{
	if (raycastHit) {
		Renderer::drawSphere(lastRayPoint, 0.02f, glm::vec4(0, 0, 1, 1));
	}

	Renderer::drawSphere(Renderer::mainCamera->getEyeRay().origin, 0.02f, glm::vec4(1, 1, 0, 1));
}

void FPSMovement::recalculate()
{
	// cache angles for front vector
	float cosYaw = glm::cos(glm::radians(yaw));
	float cosPitch = glm::cos(glm::radians(pitch));
	float sinYaw = glm::sin(glm::radians(yaw));
	float sinPitch = glm::sin(glm::radians(pitch));

	// recalculate direction variables
	front = glm::vec3(
		cosYaw * cosPitch,
		sinPitch,
		sinYaw * cosPitch);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	// now construct quaternion for mouselook
	glm::quat x = glm::angleAxis(glm::radians(-yaw), worldUp);
	glm::quat y = glm::angleAxis(glm::radians(-pitch), glm::vec3(1, 0, 0));

	if (verticality != nullptr)
	{
		gameObject->transform.setRotate(x);
		verticality->transform.setRotate(y);
	}
	else
	{
		gameObject->transform.setRotate(x * y);
	}

	// and transform me please
	gameObject->transform.setPosition(position.x, position.y, position.z);
}

void FPSMovement::respawn() {
	position = initialPosition;
}
