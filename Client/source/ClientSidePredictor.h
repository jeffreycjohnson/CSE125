#pragma once
#include "Component.h"
#include "GameObject.h"

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

class ClientSidePredictor :
	public Component
{
private:
	GameObject *body = nullptr;

	GLfloat yaw, pitch;
	float mouseSensitivity, joystickSensitivity;

	bool pastFirstTick = false;
	glm::vec2 lastMousePosition;

public:
	ClientSidePredictor(Sensitivity sensitivities);
	~ClientSidePredictor();

	void create() override;
	void update(float dt) override;

	void recalculate();
};

