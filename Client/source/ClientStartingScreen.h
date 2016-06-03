#pragma once
#include "LoadingScreen.h"

class ClientStartingScreen : public LoadingScreen {
public:
	// C++11 constructor inheritance
	using LoadingScreen::LoadingScreen;

	bool load() override;
	//bool finished() override;
};