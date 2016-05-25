#include "MainMenu.h"
#include "Config.h"
#include "UIButton.h"
#include "Input.h"

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
	auto button = new UIButton("assets/connect_button.png", 320, 240, 397, 44);
	// Screw it, use a lamba. Fuck you std::bind
	button->onClick() = [this, button]() {
		this->connect();
		button->active = false;
		std::cerr << "Connected?" << std::endl;
	};
	elements.add("Play", button);
	Input::showCursor();
}