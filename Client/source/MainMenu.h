#pragma once
#include "UI.h"
#include "Crosshair.h"

class MainMenu :
	public UIMenu
{
public:
	MainMenu();
	~MainMenu();

	// Event Listeners
	void connect();

	// Inherited from UIMenu
	void create() override;
};