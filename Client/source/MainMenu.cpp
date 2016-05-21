#include "MainMenu.h"
#include "Config.h"
#include "Input.h"

#include "GameObject.h"
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

bool MainMenu::connect()
{
	ConfigFile file("config/options.ini");

	std::string serverip = file.getString("NetworkOptions", "serverip");
	std::string port = file.getString("NetworkOptions", "port");
	auto pair = NetworkManager::InitializeClient(serverip, port);
	
	return ClientNetwork::isConnected();
}

void MainMenu::create()
{
	std::cerr << "INIT: menu. Press <ENTER> to connect to server specified in options.ini" << std::endl;
}

void MainMenu::fixedUpdate()
{
	//std::cerr << "Put-putting around in the menu!" << std::endl;
	if (Input::getKeyDown("enter")) {
		if (connect()) {
			//gameObject->removeComponent<MainMenu>(true); // Delete the menu component (causes not incrementable exception)
			this->active = false;
		}
		else {
			FATAL("Failed to connect to server when we tried. This really shouldn't be a fatal error right now, but for now let's just bail.");
		}
	}
}