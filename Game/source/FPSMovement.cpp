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
#include "KeyTarget.h"
#include "KeyActivator.h"
#include "KeyHoleTarget.h"
#include "Inventory.h"

FPSMovement::FPSMovement(int clientID, float moveSpeed, float mouseSensitivity, glm::vec3 position, glm::vec3 up, GameObject* verticality)
	: clientID(clientID), moveSpeed(moveSpeed), mouseSensitivity(mouseSensitivity), position(position), up(up), worldUp(up), verticality(verticality)
{
	this->front = glm::vec3(0, 0, -1);

	this->yaw = 0.0f;
	this->pitch = 0.0f;
	pastFirstTick = false;
	raycastHit = false;
	forward = glm::vec3(0);

	playerBoxCollider = nullptr;
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
		return;
	}

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

	// Ray cast normal debug against OBBs
	auto cameraRay = Renderer::mainCamera->getEyeRay();
	auto box = GameObject::FindByName("Player")->transform.children[0]->children[0]->gameObject->getComponent<BoxCollider>();;
	auto camhit = GameObject::SceneRoot.getComponent<OctreeManager>()->raycast(cameraRay, Octree::BOTH, 0, Octree::RAY_MAX, box);
	raycastHit = camhit.intersects;
	lastRayPoint = cameraRay.getPos(camhit.hitTime);
	lastRayPointPlusN = lastRayPoint + camhit.normal;
	// end debug

	recalculate();
	getPlayerRadii(); // Hmmm, suspicious
	raycastMouse();
}

void FPSMovement::getPlayerRadii() {
	//First we access the player's box collider
	Transform playerTrans = GameObject::FindByName("Player")->transform;
	GameObject * go = playerTrans.children[0]->children[0]->gameObject;
	playerBoxCollider = go->getComponent<BoxCollider>();

	//Then we access it's width and height, to set the player radii
	playerRadius = playerBoxCollider->getWidth() / 2.0f;
	playerHeightRadius = playerBoxCollider->getHeight() / 2.0f; // TODO: Modify the internals of BoxCollider to return correct values if OBB
}

void FPSMovement::handleHorizontalMovement(float dt) {

	// act on keyboard
	float speed = moveSpeed * dt;
	glm::vec3 normRight = glm::normalize(right);

	glm::vec3 forwardComp = ServerInput::getAxis("pitch", clientID) * forward;
	forwardComp.y = 0; // there is no such thing as "y"
	glm::vec3 sideComp = ServerInput::getAxis("roll", clientID) * right;

	//moveDir = (position + (ServerInput::getAxis("pitch", clientID) * forward)) - position;
	moveDir = forwardComp + sideComp;

	//Normalize the player's combined movement vector, and multiply it by the speed to ensure a constant velocity
	if (glm::length(moveDir) > 0)
		moveDir = glm::normalize(moveDir) * speed;

	//We raycast forward, left, and right, and update the moveDir to slide along the walls we hit
	if (oct != nullptr) {
		bool moveDirModified = true;
		int failCount = 0;

		pushOutOfAdjacentWalls(position, glm::vec3(moveDir.z, moveDir.y, -moveDir.x));
		pushOutOfAdjacentWalls(position, glm::vec3(-moveDir.z, moveDir.y, moveDir.x));

		// OKAY (verified)
		while (moveDirModified && failCount < 3) {
			moveDirModified = slideAgainstWall(position, moveDir, failCount);
			failCount++;
		}

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
	RayHitInfo moveHit = oct->raycast(moveRay, Octree::BOTH, 0, Octree::RAY_MAX, playerBoxCollider);

	//If we hit something in front of us, and it is within the player radius
	if (moveHit.intersects && moveHit.hitTime <= playerRadius && moveHit.hitTime >= 0) {

		// First, get the desired new position, assuming there was no intersection
		glm::vec3 desiredNewPos = position + (moveDir);
		glm::vec3 behindVector = glm::normalize(moveDir) * (playerRadius - moveHit.hitTime);
		float distBehindWall = std::abs(glm::dot(behindVector, moveHit.normal));

		// Project desired new position onto the wall's normal. If the dot product is negative (the position is behind
		// the normal) then, we will want to offset our desired position to be IN FRONT OF the wall normal. we take
		// the absolute value here, because we want a positive offset along the normal. We know the dot product will
		// be negative, because the raycast hit within the player's radius.

		//float distBehindWall = std::abs(glm::dot(desiredNewPos, moveHit.normal));
		glm::vec3 newPos = desiredNewPos + distBehindWall * moveHit.normal;
		moveDir = newPos - position;
		return true;

	}
	return false;
}

void FPSMovement::pushOutOfAdjacentWalls(glm::vec3 position, glm::vec3 direction) {

	// This function checks whether the player's bounding box overlaps a wall in the given direction,
	// and if so, offsets the player's position along the negative of the ray direction, so that the
	// player no longer intersects that wall. Done once before movement logic, so that we don't slowly
	// clip into walls when sliding.

	Ray sideRay(position, direction);
	RayHitInfo sideHit = oct->raycast(sideRay, Octree::BOTH, 0, Octree::RAY_MAX, playerBoxCollider);

	//If the side raycast enters a wall, we force the player back along the sideray vector to keep them out of the wall
	if (sideHit.intersects && sideHit.hitTime <= playerRadius && sideHit.hitTime >= 0) {
		moveDir += -glm::normalize(direction)*(playerRadius - sideHit.hitTime);
	}

}

void FPSMovement::handleVerticalMovement(float dt) {
	
	//This ray goes downwards from the player center, and IGNORE the player's collider

	Ray downRay(position, -worldUp);
	RayHitInfo downHit = oct->raycast(downRay, Octree::BOTH, 0, Octree::RAY_MAX, playerBoxCollider);
	bool standingOnSurface = downHit.intersects && downHit.hitTime < playerHeightRadius + 0.1f;

	Ray upRay(position, worldUp);
	RayHitInfo upHit = oct->raycast(upRay, Octree::BOTH, 0, Octree::RAY_MAX, playerBoxCollider);
	bool hitHead = upHit.intersects && upHit.hitTime < playerHeightRadius + 0.1f;

	if (standingOnSurface) {
		vSpeed = baseVSpeed;
		position.y = position.y - downHit.hitTime + playerHeightRadius;
		if (!hitHead && ServerInput::getAxis("jump", clientID) != 0) {
			vSpeed = startJumpSpeed;
			position.y += vSpeed;
		}
	}
	else {
		if (hitHead && vSpeed > 0)
			vSpeed = 0;

		vSpeed += vAccel;
		position.y += vSpeed;
	}

}

void FPSMovement::debugDraw()
{
	if (raycastHit) {
		Renderer::drawSphere(lastRayPoint, 0.25f, glm::vec4(0, 0, 1, 1)); // blue = hitpoint
		Renderer::drawSphere(lastRayPointPlusN, 0.25f, glm::vec4(1, 0, 1, 1)); // purple = normal + pt
	}

	// visualizing various vectors of importance for movement
	Renderer::drawSphere(position + forward, 0.25f, glm::vec4(1, 1, 0, 1)); // yellow
	Renderer::drawSphere(position + front, 0.125f, glm::vec4(0, 1, 0, 1)); // lime green for (front)
	Renderer::drawSphere(position, 0.25f, glm::vec4(1.000, 0.388, 0.278, 1)); // orange (position)


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

	forward = (position + front) - position;
	forward.y = 0; // pretend that the "y" axis doesn't exist
	forward = glm::normalize(forward);

	//glm::normalize(glm::cross(forward, worldUp));

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
	if (!oct) return;

	Ray ray(verticality->transform.getWorldPosition() + front, glm::vec3(front));
	auto cast = oct->raycast(ray, Octree::BuildMode::BOTH);

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
		else if (hit->getComponent<KeyTarget>() && (hit->getComponent<KeyTarget>()->isActivated() || hit->getComponent<KeyTarget>()->canBePickedUp)) {
			// pick up key
			this->gameObject->getComponent<Inventory>()->setKey(hit);
		}
		else if (hit->getComponent<KeyHoleTarget>() && this->gameObject->getComponent<Inventory>()->hasKey() ) {
			// if KeyHoleTarget matches key currently in inventory
			if (hit->getComponent<KeyHoleTarget>()->keyHoleID == this->gameObject->getComponent<Inventory>()->getKey()->getComponent<KeyActivator>()->keyHoleID) {
				std::cout << "Key matches keyHole " << hit->getComponent<KeyHoleTarget>()->keyHoleID << std::endl;
				this->gameObject->getComponent<Inventory>()->getKey()->getComponent<KeyActivator>()->trigger();
				this->gameObject->getComponent<Inventory>()->removeKey();
			}
		}
	}
}