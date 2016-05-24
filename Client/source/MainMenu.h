#pragma once
#include "UI.h"

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

