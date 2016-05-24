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
	std::cerr << "INIT: menu. Press <ENTER> to connect to server specified in options.ini" << std::endl;
	
	auto button = new UIButton("assets/connect_button.png", 320, 240);
	// Screw it, use a lamba. Fuck you std::bind
	button->onClick() = [this]() {
		this->connect();
	};
	elements.add("Play", button);
	Input::showCursor();
}