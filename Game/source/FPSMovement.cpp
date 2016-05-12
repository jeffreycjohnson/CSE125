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
	if (verticality) 
	{
		this->gameObject->addChild(verticality);
		verticality->transform.translate(worldUp * 0.6f);
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

	RayHitInfo moveHit;
	
	auto oct = GameObject::SceneRoot.getComponent<OctreeManager>();
	if (oct != nullptr) {
		if (playerRadius == 0 || playerHeightRadius == 0) {
			Transform playerTrans = GameObject::FindByName("Player")->transform;
			GameObject * go = playerTrans.children[0]->children[0]->gameObject;
			BoxCollider * b = go->getComponent<BoxCollider>();
			playerRadius = b->getWidth() / 2;
			playerHeightRadius = b->getHeight() / 2;
		}

		hitWall = false;
		glm::vec3 newMoveVec = moveDir;

		if (moveDir != glm::vec3(0)) {
			newMoveVec = handleRayCollision(position, moveDir, newMoveVec);
			newMoveVec = handleRayCollision(position, glm::vec3(moveDir.z, moveDir.y, -moveDir.x), newMoveVec);
			newMoveVec = handleRayCollision(position, glm::vec3(-moveDir.z, moveDir.y, moveDir.x), newMoveVec);

			position += newMoveVec;
			gameObject->transform.setPosition(position.x, position.y, position.z);
		}
	}
	
	glm::vec2 currMousePosition = ServerInput::mousePosition(clientID);
	glm::vec2 mouseDelta = currMousePosition - lastMousePosition;

	yaw += mouseDelta.x * mouseSensitivity;
	pitch += -1 * mouseDelta.y * mouseSensitivity;

	// can't look past certain angles
	pitch = fmaxf(-89.0f, fminf(89.0f, pitch));

	lastMousePosition = currMousePosition;

	// act on keyboard
	float speed = moveSpeed *dt;
	glm::vec3 worldFront = glm::normalize(glm::cross(worldUp, right));
	glm::vec3 normRight = glm::normalize(right);

	glm::vec3 xComp = ServerInput::getAxis("pitch", clientID) * worldFront * speed;
	glm::vec3 zComp = ServerInput::getAxis("roll", clientID) * normRight * speed;

	moveDir = xComp + zComp;
	if (!hitWall) {
		position += moveDir;
	}

	Ray downRay(position, -worldUp);
	/*RayHitInfo downHit = oct->raycast(downRay, Octree::BOTH);
	bool standingOnSurface = downHit.intersects && downHit.hitTime < playerHeightRadius + 0.1f;

	Ray upRay(position, worldUp);
	RayHitInfo upHit = oct->raycast(upRay, Octree::BOTH);
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

	if (position.y < deathFloor) {
		respawn();
	}*/

	// debug
	auto rayray = Renderer::mainCamera->getEyeRay();
	auto box = GameObject::FindByName("Player")->transform.children[0]->children[0]->gameObject->getComponent<BoxCollider>();;
	auto shit = GameObject::SceneRoot.getComponent<OctreeManager>()->raycast(rayray, Octree::BOTH, 0, Octree::RAY_MAX, box);
	raycastHit = shit.intersects;
	lastRayPoint = rayray.getPos(shit.hitTime);
	// end debug
	
	recalculate();

	raycastMouse();
}

glm::vec3 FPSMovement::handleRayCollision(glm::vec3 position, glm::vec3 castDirection, glm::vec3 moveDirection) 
{
	auto oct = GameObject::SceneRoot.getComponent<OctreeManager>();
	Ray moveRay(position, castDirection);
	RayHitInfo moveHit = oct->raycast(moveRay, Octree::BOTH);
	glm::vec3 newMoveVec = moveDirection;
	if (moveHit.intersects && moveHit.hitTime <= playerRadius && moveHit.hitTime >= 0) {
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
		Renderer::drawSphere(lastRayPoint, 0.25f, glm::vec4(0, 0, 1, 1));
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

	if (!setVerticalityForward) {
		verticality->transform.translate(-right * 0.35f);
		setVerticalityForward = true;
	}

	// now construct quaternion for mouselook
	glm::vec3 worldFront = glm::normalize(glm::cross(worldUp, right));
	glm::vec3 frontUp = glm::dot(front, worldUp) * worldUp;

	glm::quat x = glm::inverse(glm::quat(glm::lookAt(gameObject->transform.getWorldPosition(), gameObject->transform.getWorldPosition() + worldFront, worldUp)));
	glm::quat y = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));

	glm::quat xy = glm::inverse(glm::quat(glm::lookAt(verticality->transform.getWorldPosition(), verticality->transform.getWorldPosition() + front, worldUp)));

	if (verticality != nullptr)
	{
		gameObject->transform.setRotate(x);
		verticality->transform.setRotate(y);
	}
	else
	{
		gameObject->transform.setRotate(xy);
	}

	// and transform me please
	gameObject->transform.setPosition(position.x, position.y, position.z);
}

void FPSMovement::respawn() {
	position = initialPosition;
}

void FPSMovement::raycastMouse()
{
	return; // Jason said to do this and I trust him
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
		
	}
}