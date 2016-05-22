#pragma once
#include "LoadingScreen.h"

class ClientStartingScreen : public LoadingScreen {
public:
	// C++11 constructor inheritance
	using LoadingScreen::LoadingScreen;

	void load() override;
	void finished() override;
};