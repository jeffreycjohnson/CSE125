#include "ClientSidePredictor.h"

#include "Input.h"

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtx/string_cast.hpp>

ClientSidePredictor::ClientSidePredictor(Sensitivity sensitivities)
	: mouseSensitivity(sensitivities.mouseSensitivity), joystickSensitivity(sensitivities.joystickSensitivity)
{
	this->yaw = 0.0f;
	this->pitch = 0.0f;
}


ClientSidePredictor::~ClientSidePredictor()
{
}

void ClientSidePredictor::create()
{
}

void ClientSidePredictor::update(float dt)
{
	if (gameObject->transform.getParent())
	{
		body = gameObject->transform.getParent()->gameObject;
	}
	else
	{
		return;
	}

	if (!pastFirstTick)
	{
		pastFirstTick = true;

		glm::vec2 currMousePosition = Input::mousePosition();
		lastMousePosition = currMousePosition;
		return;
	}

	glm::vec2 currMousePosition = Input::mousePosition();

	glm::vec2 mouseDelta = (currMousePosition - lastMousePosition) * mouseSensitivity;
	glm::vec2 joyDelta = glm::vec2(Input::getAxis("lookright"), Input::getAxis("lookforward")) * joystickSensitivity;
	glm::vec2 lookDelta = (mouseDelta + joyDelta) * dt;

	yaw += lookDelta.x;
	pitch += -1 * lookDelta.y;

	// can't look past certain angles
	pitch = fmaxf(-89.0f, fminf(89.0f, pitch));

	lastMousePosition = currMousePosition;
}

void ClientSidePredictor::recalculate()
{

	// cache angles for front vector
	float cosYaw = glm::cos(glm::radians(yaw));
	float cosPitch = glm::cos(glm::radians(pitch));
	float sinYaw = glm::sin(glm::radians(yaw));
	float sinPitch = glm::sin(glm::radians(pitch));

	// recalculate direction variables
	glm::vec3 front = glm::normalize(glm::vec3(
		cosYaw * cosPitch,
		sinPitch,
		sinYaw * cosPitch));
	glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::normalize(glm::cross(right, front));

	glm::vec3 worldFront = glm::normalize(glm::cross(glm::vec3(0,1,0), right));
	glm::mat4 newLookAt = glm::lookAt(gameObject->transform.getWorldPosition(), gameObject->transform.getWorldPosition() + worldFront, glm::vec3(0, 1, 0));

	glm::quat x = glm::inverse(glm::quat(newLookAt));
	glm::quat y = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));

	body->transform.setRotate(x);
	gameObject->transform.setRotate(y);
}
