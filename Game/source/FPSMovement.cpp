#include "FPSMovement.h"

#include <iostream>

#include <glm/glm/gtc/matrix_transform.hpp>

#include "GameObject.h"
#include "ServerInput.h"
#include "Renderer.h"
#include "Timer.h"

const float SPEED = 3.0f;

FPSMovement::FPSMovement(int clientId, float moveSpeed, float mouseSensitivity, glm::vec3 position, glm::vec3 up, GameObject* verticality)
	: clientId(clientId), moveSpeed(moveSpeed), mouseSensitivity(mouseSensitivity), position(position), up(up), worldUp(up), verticality(verticality)
{
	this->front = glm::vec3(0, 0, -1);

	this->yaw = -90.0f;
	this->pitch = 0.0f;
}

void FPSMovement::create()
{
	if (verticality) this->gameObject->addChild(verticality);
	
	recalculate();
}

void FPSMovement::fixedUpdate()
{
	auto dt = Timer::fixedTimestep;
	
	// act on mouse
	glm::vec2 currMousePosition = ServerInput::mousePosition(clientId);
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

	position += ServerInput::getAxis("roll", clientId) * worldFront * speed;
	position += ServerInput::getAxis("pitch", clientId) * normRight * speed;
	
	recalculate();
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