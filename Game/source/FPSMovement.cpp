#include "FPSMovement.h"

#include <iostream>
#include <typeinfo>

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtx/string_cast.hpp>

#include "Camera.h"
#include "Collider.h"
#include "GameObject.h"
#include "Input.h"
#include "OctreeManager.h"
#include "Renderer.h"
#include "Timer.h"
#include "OctreeManager.h"
#include "Camera.h"
#include "Collider.h"
#include "Collision.h"
#include "BoxCollider.h"
#include "Config.h"
#include "Sound.h"
#include "Crosshair.h"

#include "PressButton.h"
#include "KeyTarget.h"
#include "FixedKeyTarget.h"
#include "KeyActivator.h"
#include "KeyHoleTarget.h"
#include "Inventory.h"

float FPSMovement::baseHSpeed = 4.0f;
float FPSMovement::baseVSpeed = -0.2f;
float FPSMovement::startJumpSpeed = 10.f;
float FPSMovement::vAccel = -27.5f;
float FPSMovement::deathFloor = -20.0f;
float FPSMovement::interactDistance = 5.0f;

FPSMovement::FPSMovement(
	int clientID, Sensitivity sensitivites,
	glm::vec3 initPosition, glm::vec3 upVector,
	GameObject* verticality)
	: clientID(clientID), 
	  mouseSensitivity(sensitivites.mouseSensitivity), joystickSensitivity(sensitivites.joystickSensitivity), 
	  position(initPosition), initialPosition(initPosition), up(upVector), worldUp(upVector),
	  verticality(verticality)
{
	this->front = glm::vec3(0, 0, -1);

	this->deathTimer = 0.0;
	this->deaded = false;
	this->justDeaded = false;

	ConfigFile file("config/options.ini");
	this->deathDefaultTime = file.getFloat("GameSettings", "deathDefaultTime");

	this->yaw = 0.0f;
	this->pitch = 0.0f;
	pastFirstTick = false;
	raycastHit = false;
	forward = glm::vec3(0);
	floor = nullptr;
	jumpSound = nullptr;
	oct = GameObject::SceneRoot.getComponent<OctreeManager>();
	ASSERT(oct != nullptr, "ERROR: Octree is a nullptr");
}

void FPSMovement::loadGameSettings(ConfigFile & file)
{
	if (file.getBool("GameSettings", "enableOverride")) {
		baseHSpeed = file.getFloat("GameSettings", "baseHSpeed");
		baseVSpeed = file.getFloat("GameSettings", "baseVSpeed");
		startJumpSpeed = file.getFloat("GameSettings", "startJumpSpeed");
		vAccel = file.getFloat("GameSettings", "vAccel");
		deathFloor = file.getFloat("GameSettings", "deathFloor");
		interactDistance = file.getFloat("GameSettings", "interactDistance");
	}
}

void FPSMovement::create()
{
	if (verticality) 
	{
		this->gameObject->addChild(verticality);
		verticality->transform.translate(worldUp * 0.6f);
		verticality->transform.translate(front * 0.25f);
	}

	auto player = this->gameObject->findChildByName("Player");
	ASSERT(player != nullptr, "Could not locate 'Player' node from specified player model.");
	auto colliders = player->findChildByName("Colliders");
	auto boxes = player->findChildByName("BoxCollider");

	// Assimp turns spaces into underscores
	playerBoxCollider = player->findChildByName("Colliders")->findChildByName("BoxCollider_body")->getComponent<BoxCollider>();
	feetCollider = player->findChildByName("Colliders")->findChildByName("BoxCollider_feet")->getComponent<BoxCollider>();

	ConfigFile soundConfig("config/sounds.ini");
	// Add sounds
	testBroadcastSound = Sound::affixSoundToDummy(gameObject, new Sound("voice_windows_10", false, false, 1.0, true, Sound::BROADCAST));
	float jumpVol = soundConfig.getFloat("jumpsound", "volume");
	float deathVol = soundConfig.getFloat("deathsound", "volume");
	float landVol = soundConfig.getFloat("landsound", "volume");
	//float emoteVol = soundConfig.getFloat("robotemote", "volume");

	jumpSound = Sound::affixSoundToDummy(gameObject, new Sound("jumpsound", false, false, jumpVol, true, Sound::SOUND_EFFECT));
	deathRattle = Sound::affixSoundToDummy(gameObject, new Sound("deathsound", false, false, deathVol, true, Sound::SOUND_EFFECT));
	landSound = Sound::affixSoundToDummy(gameObject, new Sound("landsound", false, false, landVol, true));
	//emoteSound = Sound::affixSoundToDummy(gameObject, new Sound("emotesound", false, false, landVol, true));

	//Input::hideCursor();
	recalculate();
}

void FPSMovement::fixedUpdate()
{
	auto dt = Timer::fixedTimestep;

	if (!pastFirstTick)
	{
		pastFirstTick = true;

		glm::vec2 currMousePosition = Input::mousePosition(clientID);
		lastMousePosition = currMousePosition;
		return;
	}

	if (deaded) {
		deathTimer -= dt;
		auto data = std::vector<char>();
		if (justDeaded) {
			justDeaded = false;
			std::cout << "Sending DEATH to " << clientID << std::endl;
			NetworkManager::PostMessage(data, PLAYER_HAS_DIED_EVENT, clientID, clientID);
		}
		else if (deathTimer <= 0.0f) {
			std::cout << "Sending LIFE to " << clientID << std::endl;
			NetworkManager::PostMessage(data, PLAYER_HAS_RESPAWNED_EVENT, clientID, clientID);
			deaded = false;
			deathTimer = 0.0f;
		}
	}

	glm::vec2 currMousePosition = Input::mousePosition(clientID);
	
	glm::vec2 mouseDelta = (currMousePosition - lastMousePosition) * mouseSensitivity;
	glm::vec2 joyDelta = glm::vec2(Input::getAxis("lookright", clientID), Input::getAxis("lookforward", clientID)) * joystickSensitivity;
	glm::vec2 lookDelta = (mouseDelta + joyDelta) * dt;

	yaw += lookDelta.x;
	pitch += -1 * lookDelta.y;

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

	if ( Input::getButtonDown("enter", clientID) ) {
		std::cerr << "should queue broadcast sound" << std::endl;
		testBroadcastSound->play();
	}

	recalculate();
	getPlayerRadii();
	raycastMouse();

	// Robot chirp
	//robotEmote();

	if(Input::getAxis("respawn", clientID) > 0.0f)
		respawn();
}

void FPSMovement::robotEmote() {
	float emoteButton = Input::getAxis("emote", clientID);
	if (emoteButton) emoteSound->play();
}

void FPSMovement::getPlayerRadii() {
	if (playerBoxCollider) {
		//Then we access it's width and height, to set the player radii
		//playerRadius = playerBoxCollider->getWidth() / 2.0f;
		float w = playerBoxCollider->getWidth();
		float d = playerBoxCollider->getHeight();
		float theta = std::atanf(d / w);
		playerRadius = (d / std::sin(theta)) / 2.0f; // Divide by two b/c this is radius

		playerHeightRadius = playerBoxCollider->getDepth() / 2.0f; // TODO: Modify the internals of BoxCollider to return correct values if OBB
	}
	if (feetCollider) {
		float fw = feetCollider->getWidth();
		float fd = feetCollider->getHeight();
		float ftheta = std::atanf(fd / fw);
		footRadius = (fd / std::sin(ftheta)) / 2.0f; // Divide by two b/c this is radius
	}
}

void FPSMovement::handleHorizontalMovement(float dt) {

	// act on keyboard
	float speed = baseHSpeed * dt;
	glm::vec3 normRight = glm::normalize(right);

	glm::vec3 forwardComp = Input::getAxis("forward", clientID) * forward;
	forwardComp.y = 0; // there is no such thing as "y"
	glm::vec3 sideComp = Input::getAxis("right", clientID) * right;

	//moveDir = (position + (ServerInput::getAxis("pitch", clientID) * forward)) - position;
	moveDir = forwardComp + sideComp;

	//Normalize the player's combined movement vector, and multiply it by the speed to ensure a constant velocity
	if (glm::length(moveDir) > 0)
	{
		moveDir = glm::normalize(moveDir) * speed;
	}

	//We raycast forward, left, and right, and update the moveDir to slide along the walls we hit
	if (oct != nullptr) {
		bool moveDirModified = true;
		int failCount = 0;

		pushOutOfAdjacentWalls(position, glm::vec3(moveDir.z, moveDir.y, -moveDir.x));
		pushOutOfAdjacentWalls(position, glm::vec3(-moveDir.z, moveDir.y, moveDir.x));

		// OKAY (verified)
		while (moveDirModified && failCount < 3) {
			// raycasting directly from position is too high, we can jump above objects
			moveDirModified = slideAgainstWall(position, moveDir, failCount);
			if (!moveDirModified && playerBoxCollider) {
				// Check our feet
				moveDirModified = slideAgainstWall(position - glm::vec3(0, -playerBoxCollider->getHeight() / 2, 0), moveDir, failCount);
			}
			if (!moveDirModified && playerBoxCollider) {
				// Check our head
				moveDirModified = slideAgainstWall(position - glm::vec3(0, playerBoxCollider->getHeight() / 2, 0), moveDir, failCount);
			}
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

	// We want to ignore the player's collider & the collider that we are standing on, if it exists
	RayHitInfo moveHit = oct->raycast(moveRay, Octree::BOTH, 0, Octree::RAY_MAX, { playerBoxCollider, floor });

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
		glm::vec3 newPos = desiredNewPos + distBehindWall * moveHit.normal;
		moveDir = newPos - position;
		return true;

	}
	return false;
}

void FPSMovement::pushOutOfAdjacentWalls(glm::vec3 pos, glm::vec3 direction) {

	// This function checks whether the player's bounding box overlaps a wall in the given direction,
	// and if so, offsets the player's position along the negative of the ray direction, so that the
	// player no longer intersects that wall. Done once before movement logic, so that we don't slowly
	// clip into walls when sliding.

	Ray sideRay(pos, direction);
	RayHitInfo sideHit = oct->raycast(sideRay, Octree::BOTH, 0, Octree::RAY_MAX, { playerBoxCollider });

	//If the side raycast enters a wall, we force the player back along the sideray vector to keep them out of the wall
	if (sideHit.intersects && sideHit.hitTime <= playerRadius && sideHit.hitTime >= 0) {
		moveDir += -glm::normalize(direction)*(playerRadius - sideHit.hitTime);
	}

}

void FPSMovement::handleVerticalMovement(float dt) {
	
	bool previouslyStandingOnSurface = standingOnSurface;
	standingOnSurface = false;
	//These rays goes downwards from the player, and IGNORES the player's collider
	//Having 4 rays gives is more precision for knowing in the player is partially standing on a surface
	checkOnSurface(position, -worldUp);

	if (!standingOnSurface)
		checkOnSurface(position + glm::vec3(footRadius, 0, footRadius), -worldUp);
	if (!standingOnSurface)
		checkOnSurface(position + glm::vec3(footRadius, 0, -footRadius), -worldUp);
	if (!standingOnSurface)
		checkOnSurface(position + glm::vec3(-footRadius, 0, footRadius), -worldUp);
	if (!standingOnSurface)
		checkOnSurface(position + glm::vec3(-footRadius, 0, -footRadius), -worldUp);

	if (!previouslyStandingOnSurface && standingOnSurface) {
		landSound->play();
	}

	//This ray goes straight up from the player's center
	Ray upRay(position, worldUp);
	RayHitInfo upHit = oct->raycast(upRay, Octree::BOTH, 0, playerBoxCollider->getHeight(), { playerBoxCollider });
	bool hitHead = upHit.intersects && upHit.hitTime < playerHeightRadius + 0.1f;

	//After we release the jump button, we can not jump again
	if (Input::getAxis("jump", clientID) == 0)
		justJumped = false;

	//If we are currently on a surface, snap us to the player's standing height
	if (standingOnSurface) {
		vSpeed = 0;//baseVSpeed;
		position.y = position.y - downHit.hitTime + playerHeightRadius;

		//If nothin is on our head, and we try to jump, and we aren't holding space from a previous jump
		if (!hitHead && Input::getAxis("jump", clientID) != 0 && !justJumped) {
			vSpeed = startJumpSpeed;
			position.y += vSpeed * dt * 2;
			justJumped = true;

			if (jumpSound) {
				jumpSound->play();
			}

		}
	}
	//If we're falling or flying upwards, accellerate down
	else {
		//If we hit our head while jumping, stop all upward momentum
		if (hitHead && vSpeed > 0)
			vSpeed = 0;

		vSpeed += vAccel * dt;
		position.y += vSpeed * dt;
	}

}

void FPSMovement::checkOnSurface(glm::vec3 position, glm::vec3 direction) {

	//We raycast downwards from our position
	Ray downRay(position, direction);
	RayHitInfo newDownHit = oct->raycast(downRay, Octree::BOTH, 0, playerBoxCollider->getHeight(), { playerBoxCollider, feetCollider });

	//If standingOnSurface is already true, or this downHit has determined that we are on a surface, then set standingOnSurface to true
	standingOnSurface = standingOnSurface || (newDownHit.intersects && newDownHit.hitTime < playerHeightRadius + 0.1f);
	floor = newDownHit.collider;

	//If we just found that we're standing on a surface, then the global downHit is set to this one
	if (newDownHit.intersects && newDownHit.hitTime < playerHeightRadius + 0.1f) {
		downHit = newDownHit;
	}
	else {
		floor = nullptr;
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
	Renderer::drawSphere(verticality->transform.getWorldPosition(), 0.5f, glm::vec4(1, 0.25, 1, 1)); // purplish - verticality pos

	Renderer::drawSphere(verticality->transform.getWorldPosition() + front, 0.125f, glm::vec4(0, 1, 0, 1)); // lime green for (front)
	Renderer::drawSphere(position, 0.25f, glm::vec4(1.000, 0.388, 0.278, 1)); // orange (position)

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
	deathRattle->play();
	deaded = true;
	justDeaded = true;

	deathTimer = deathDefaultTime;
	std::cout << "deathTimer set " << deathTimer << std::endl;
	position = initialPosition;
	vSpeed = 0;
}

void FPSMovement::raycastMouse()
{
	if (!oct) return;

	Ray ray(verticality->transform.getWorldPosition(), glm::vec3(front));
	auto cast = oct->raycast(ray, Octree::DYNAMIC_ONLY, 0, FPSMovement::interactDistance, { playerBoxCollider }, false);

	if (!cast.intersects) {
		//if (Renderer::crosshair->getState() != CrosshairNetworkData::CrosshairState::DEFAULT) {
			Crosshair::setState(CrosshairNetworkData::CrosshairState::DEFAULT, clientID);
		//}
		return; // ^ Doubt this has anything to do with it, but it could help with frame-rate lag?
	}

	GameObject *hit = cast.collider->gameObject->transform.getParent()->getParent()->gameObject;
	GameObject *phit = hit->transform.getParent() ? hit->transform.getParent()->gameObject : hit;

	// Handle crosshair changes if looking at objects of interest
	if (hit->getComponent<PressButton>() || phit->getComponent<PressButton>() ||
		(hit->getComponent<FixedKeyTarget>() && (hit->getComponent<FixedKeyTarget>()->isActivated() || hit->getComponent<FixedKeyTarget>()->canBePickedUp)) ||
		(hit->getComponent<KeyTarget>() && (hit->getComponent<KeyTarget>()->isActivated() || hit->getComponent<KeyTarget>()->canBePickedUp))
		|| hit->getComponent<KeyHoleTarget>())
	{
		//if (Renderer::crosshair->getState() != CrosshairNetworkData::CrosshairState::INTERACTIVE) {
			Crosshair::setState(CrosshairNetworkData::CrosshairState::INTERACTIVE, clientID);
		//}
	}
	else {
		//if (Renderer::crosshair->getState() != CrosshairNetworkData::CrosshairState::DEFAULT) {
			Crosshair::setState(CrosshairNetworkData::CrosshairState::DEFAULT, clientID);
		//}
	}
	float clickf = Input::getAxis("click", clientID);

	if (clickf && !justClicked)
	{
		justClicked = true;
		std::cout << "hit " << hit->getName() << std::endl;
		std::cout << "phit " << phit->getName() << std::endl;
		if (hit->getComponent<PressButton>())
		{
			hit->getComponent<PressButton>()->trigger();
		}
		else if (phit->getComponent<PressButton>())
		{
			phit->getComponent<PressButton>()->trigger();
		}
		else if ((hit->getComponent<FixedKeyTarget>() && (hit->getComponent<FixedKeyTarget>()->isActivated() || hit->getComponent<FixedKeyTarget>()->canBePickedUp)) ||
			(hit->getComponent<KeyTarget>() && (hit->getComponent<KeyTarget>()->isActivated() || hit->getComponent<KeyTarget>()->canBePickedUp))) 
		{
			// pick up key
			this->gameObject->getComponent<Inventory>()->setKey(hit);
		}
		else if (hit->getComponent<KeyHoleTarget>() && this->gameObject->getComponent<Inventory>()->hasKey() ) 
		{
			// if KeyHoleTarget matches key currently in inventory
			if (hit->getComponent<KeyHoleTarget>()->keyHoleID == this->gameObject->getComponent<Inventory>()->getKey()->getComponent<KeyActivator>()->keyHoleID) {
				std::cout << "Key matches keyHole " << hit->getComponent<KeyHoleTarget>()->keyHoleID << std::endl;
				this->gameObject->getComponent<Inventory>()->getKey()->getComponent<KeyActivator>()->trigger();
				this->gameObject->getComponent<Inventory>()->removeKey();
			}
		}
	}
	else if (clickf <= 0.01 && justClicked)
	{
		justClicked = false;
	}

	bool pressHack = Input::getButtonDown("hack", clientID);
	bool pressUnhack = Input::getButtonDown("unhack", clientID);


	Target* targ = hit->getComponent<Target>() ? hit->getComponent<Target>() : phit->getComponent<Target>();
	if (targ)
	{
		if (pressHack)
		{
			targ->hack();
		}
		else if (pressUnhack)
		{
			targ->unhack();
		}
	}
}
