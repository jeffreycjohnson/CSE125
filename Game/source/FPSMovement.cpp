#include "FPSMovement.h"

#include <iostream>

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtx/string_cast.hpp>

#include "Camera.h"
#include "Collider.h"
#include "GameObject.h"
#include "Input.h"
#include "OctreeManager.h"
#include "ServerInput.h"
#include "Renderer.h"
#include "Timer.h"
#include "OctreeManager.h"
#include "Camera.h"
#include "Collider.h"
#include "Collision.h"
#include "BoxCollider.h"
#include "ServerInput.h"
#include "Config.h"

#include "PressButton.h"

FPSMovement::FPSMovement(int clientID, float moveSpeed, float mouseSensitivity, glm::vec3 position, glm::vec3 up, GameObject* verticality)
	: clientID(clientID), moveSpeed(moveSpeed), mouseSensitivity(mouseSensitivity), position(position), up(up), worldUp(up), verticality(verticality)
{
	this->front = glm::vec3(0, 0, -1);

	this->yaw = 0.0f;
	this->pitch = 0.0f;
	pastFirstTick = false;
	raycastHit = false;

	oct = GameObject::SceneRoot.getComponent<OctreeManager>();
	if (oct == nullptr) {
		throw "ERROR: Octree is a nullptr";
	}
}

void FPSMovement::create()
{
	if (verticality) 
	{
		this->gameObject->addChild(verticality);
		verticality->transform.translate(worldUp * 0.6f);
		verticality->transform.translate(front * 0.35f);
	}

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

		//On the very first tick we get the radii of the player's bounding boxes
		getPlayerRadii();
		return;
	}

	RayHitInfo moveHit;

	glm::vec2 currMousePosition = ServerInput::mousePosition(clientID);
	glm::vec2 mouseDelta = currMousePosition - lastMousePosition;

	yaw += mouseDelta.x * mouseSensitivity;
	pitch += -1 * mouseDelta.y * mouseSensitivity;

	// can't look past certain angles
	pitch = fmaxf(-89.0f, fminf(89.0f, pitch));

	lastMousePosition = currMousePosition;

	//Gets the inputs and updates position accordingly
	handleHorizontalMovement(dt);

	//Handles the player falling off surfaces and jumping
	handleVerticalMovement(dt);

	//If they fall below the "death floor", they die and respawn
	if (position.y < deathFloor) {
		respawn();
	}
	
	recalculate();

	raycastMouse();
}

void FPSMovement::getPlayerRadii() {
	//First we access the player's box collider
	Transform playerTrans = GameObject::FindByName("Player")->transform;
	GameObject * go = playerTrans.children[0]->children[0]->gameObject;
	BoxCollider * b = go->getComponent<BoxCollider>();

	//Then we access it's width and height, to set the player radii
	playerRadius = b->getWidth() / 2;
	playerHeightRadius = b->getHeight() / 2;
}

void FPSMovement::handleHorizontalMovement(float dt) {
	// act on keyboard
	float speed = moveSpeed * dt;
	glm::vec3 worldFront = glm::normalize(glm::cross(worldUp, right));
	glm::vec3 normRight = glm::normalize(right);

	//The move dir is the combined x and z movement components
	glm::vec3 xComp = ServerInput::getAxis("pitch", clientID) * worldFront;
	glm::vec3 zComp = ServerInput::getAxis("roll", clientID) * normRight;
	moveDir = xComp + zComp;

	//Normalize the player's combined movement vector, and multiply it by the speed to ensure a constant velocity
	if (glm::length(moveDir) > 0)
		moveDir = glm::normalize(moveDir) * speed;

	//We raycast forward, left, and right, and update the moveDir to slide along the walls we hit
	if (oct != nullptr) {
		bool moveDirModified = true;
		int failCount = 0;
		while (moveDirModified && failCount < 3) {
			moveDirModified = slideAgainstWall(position, moveDir, failCount);
			failCount++;
		}

		//std::cout << failCount << std::endl;
		//We try 3 times to change moveDir, if our final try was inside a wall we don't move
		if (moveDirModified && failCount == 3) {
			moveDir = glm::vec3(0);
		}
	}

	//Update the position with the new movement vector
	position += moveDir;
}

bool FPSMovement::slideAgainstWall(glm::vec3 position, glm::vec3 castDirection, int failCount)
{
	//We raycast in the given direction
	Ray moveRay(position, castDirection);
	RayHitInfo moveHit = oct->raycast(moveRay, Octree::BOTH);

	//If we hit something in front of us, and it is within the player radius
	if (moveHit.intersects && moveHit.hitTime <= playerRadius && moveHit.hitTime >= 0) {
		//Dexter's Magic Math
		glm::vec3 desiredNewPos = position + moveDir;
		glm::vec3 behindVector = glm::normalize(desiredNewPos - position) * (playerRadius - moveHit.hitTime);
		float distBehindWall = std::abs(glm::dot(behindVector, moveHit.normal));
		glm::vec3 newPos = desiredNewPos + distBehindWall * moveHit.normal;
		moveDir = newPos - position;
		return true;
	}
	else /*if(failCount != 1) */{
		handleSideCollisions(position, glm::vec3(moveDir.z, moveDir.y, -moveDir.x));
		handleSideCollisions(position, glm::vec3(-moveDir.z, moveDir.y, moveDir.x));
	}
	return false;
}

void FPSMovement::handleSideCollisions(glm::vec3 position, glm::vec3 direction) {
	Ray sideRay(position, direction);
	RayHitInfo sideHit = oct->raycast(sideRay, Octree::BOTH);
	
	//PROBLEM: This means that the instant rotation that occurs when we hit a wall creates a jump when the side ray is suddenly way inside the wall
	if (sideHit.intersects && sideHit.hitTime <= playerRadius && sideHit.hitTime >= 0) {
		moveDir += -glm::normalize(direction)*(playerRadius - sideHit.hitTime);

		/*glm::vec3 desiredNewPos = position + moveDir;
		float dist = glm::dot(desiredNewPos, sideHit.normal);
		if (dist < playerRadius) {
			glm::vec3 q = desiredNewPos + playerRadius*sideHit.normal;
			moveDir = q - position;
		}*/
		//glm::vec3 behindVector = glm::normalize(desiredNewPos - position) * (playerRadius - sideHit.hitTime);
		//float distBehindWall = std::abs(glm::dot(behindVector, sideHit.normal));
		//glm::vec3 newPos = desiredNewPos + distBehindWall * sideHit.normal;
		//moveDir = newPos - position;
	}

}

void FPSMovement::handleVerticalMovement(float dt) {
	//This ray goes downwards from the player center
	Ray downRay(position, -worldUp);
	RayHitInfo downHit = oct->raycast(downRay, Octree::BOTH);
	//If the player has a collider below them that is closer than half their height + 0.1f, then they are standing on it
	bool standingOnSurface = downHit.intersects && downHit.hitTime < playerHeightRadius + 0.1f;

	//This ray goes up from the player center
	Ray upRay(position, worldUp);
	RayHitInfo upHit = oct->raycast(upRay, Octree::BOTH);
	//If the player has a collider above them that is closer than half their height + 1.0f, then they hit their head on it
	bool hitHead = upHit.intersects && upHit.hitTime < playerHeightRadius + 0.1f;

	if (standingOnSurface) {
		//If they are standing on a surface, their vertical movement gets reset to the base falling speed
		vSpeed = baseVSpeed;
		//Their y position is snapped to the surface they are on, plus half their height
		position.y = position.y - downHit.hitTime + playerHeightRadius;

		//They can only jump if they don't have something on top of them
		if (!hitHead && ServerInput::getAxis("jump", clientID) != 0) {
			vSpeed = startJumpSpeed;
			position.y += vSpeed*dt;
		}
	}
	else {
		//If they are moving up and hit their head, they stop moving up
		if (hitHead && vSpeed > 0)
			vSpeed = 0;

		//If they are not on a surface they are either moving up or falling
		vSpeed += vAccel;
		position.y += vSpeed*dt;
	}
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
	front = glm::normalize(glm::vec3(
		cosYaw * cosPitch,
		sinPitch,
		sinYaw * cosPitch));
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	// now construct quaternion for mouselook
	glm::vec3 worldFront = glm::normalize(glm::cross(worldUp, right));

	glm::mat4 newLookAt = glm::lookAt(gameObject->transform.getWorldPosition(), gameObject->transform.getWorldPosition() + worldFront, worldUp);
	glm::quat x = glm::inverse(glm::quat(newLookAt));
	glm::quat y = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));

	glm::vec3 xrot = glm::eulerAngles(gameObject->transform.getRotation());
	//std::cout << "X in rot: " << xrot.x * 180 / 3.141 << ", " << xrot.y << ", " << xrot.z << std::endl;

	gameObject->transform.setRotate(x);
	verticality->transform.setRotate(y);

	// and transform me please
	gameObject->transform.setPosition(position.x, position.y, position.z);

	glm::vec3 rot = glm::eulerAngles(gameObject->transform.getRotation());
}

void FPSMovement::respawn() {
	position = initialPosition;
}

void FPSMovement::raycastMouse()
{
	auto octreeManager = GameObject::SceneRoot.getComponent<OctreeManager>();
	if (!octreeManager) return;

	Ray ray(verticality->transform.getWorldPosition() + front, glm::vec3(front));
	auto cast = octreeManager->raycast(ray, Octree::BuildMode::DYNAMIC_ONLY);

	if (!cast.intersects) return;

	if (ServerInput::getAxis("aim", clientID))
	{
		std::cout << "BUTTON TRIGGER" << std::endl;
		GameObject *hit = cast.collider->gameObject->transform.getParent()->getParent()->gameObject;
		std::cout << hit->getName() << std::endl;
		if (hit->getComponent<PressButton>())
		{
			hit->getComponent<PressButton>()->trigger();
		}
	}
}