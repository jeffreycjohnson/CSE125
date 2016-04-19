#include "FPSMovement.h"

#include <iostream>

#include <glm/glm/gtc/matrix_transform.hpp>

#include "GameObject.h"
#include "Input.h"
#include "Renderer.h"
#include "Timer.h"

const float SPEED = 3.0f;

FPSMovement::FPSMovement(float moveSpeed, float mouseSensitivity, glm::vec3 position, glm::vec3 up) 
	: moveSpeed(moveSpeed), mouseSensitivity(mouseSensitivity), position(position), up(up), worldUp(up)
{
	this->front = glm::vec3(0, 0, -1);

	this->yaw = -90.0f;
	this->pitch = 0.0f;
}

void FPSMovement::create()
{
	Input::hideCursor();

	recalculate();
}

void FPSMovement::fixedUpdate()
{
	auto time = Timer::fixedTimestep;

	/*
	1. poll clients for recv
	2. proc msgs
	3. physics
	4. game logic
	5. send msg
	*/

}

void FPSMovement::update(float dt)
{
	// act on mouse
	glm::vec2 currMousePosition = Input::mousePosition();
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

	position += Input::getAxis("roll") * worldFront * speed;
	position += Input::getAxis("pitch") * normRight * speed;

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
	gameObject->transform.setRotate(x * y);

	// and transform me please
	gameObject->transform.setPosition(position.x, position.y, position.z);
}