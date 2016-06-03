#include "MainMenu.h"
#include "Config.h"
#include "UIButton.h"
#include "Input.h"
#include "Renderer.h"

#include "GameObject.h"
#include "Crosshair.h"
#include "ClientNetwork.h"
#include "NetworkManager.h"
#include "NetworkUtility.h"

#include <iostream>

MainMenu::MainMenu()
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::connect()
{
	ConfigFile file("config/options.ini");

	std::string serverip = file.getString("NetworkOptions", "serverip");
	std::string port = file.getString("NetworkOptions", "port");
	auto pair = NetworkManager::InitializeClient(serverip, port);
	
	//return ClientNetwork::isConnected();
}

void MainMenu::create()
{	
	auto button = new UIButton("assets/connect_button.png", Renderer::getWindowWidth() / 2, Renderer::getWindowHeight() / 2, 397, 44);
	Renderer::crosshair->hide();

	// Screw it, use a lambda. Fuck you std::bind
	button->onClick() = [this, button]() {
		button->active = false;
		Input::hideCursor();
		if (Renderer::crosshair != nullptr) {
			Renderer::crosshair->show();
		}
		this->connect();
	};
	elements.add("Play", button);
	Input::showCursor();
}