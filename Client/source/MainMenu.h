#pragma once
#include "Component.h"

class MainMenu :
	public Component
{
public:
	MainMenu();
	~MainMenu();

	bool connect();

	// Inherited from Component
	void create() override;
	void fixedUpdate() override;
};

