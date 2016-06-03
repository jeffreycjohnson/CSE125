#include "ClientStartingScreen.h"
#include "ObjectLoader.h"
#include "Config.h"
#include "NetworkManager.h"
#include "GameObject.h"
#include "MainMenu.h"
#include "Crosshair.h"
#include "Renderer.h"
#include <iostream>

bool ClientStartingScreen::load()
{
	// Called on mian thread (GL calls do not fail)
	ConfigFile file("config/options.ini");

	std::string serverip = file.getString("NetworkOptions", "serverip");
	std::string port = file.getString("NetworkOptions", "port");
	bool connectOnStart = file.getBool("GameSettings", "skipMenu");

	if (connectOnStart) {
		auto pair = NetworkManager::InitializeClient(serverip, port);
	}
	else {
		//NetworkManager::InitializeOffline(); // this doesn't do anything anyway
	}

	// cache all meshes
	GameObject *scene = loadScene(file.getString("GameSettings", "level"), false, false);
	scene->destroy();
	delete scene;

	GameObject *player = loadScene(file.getString("GameSettings", "player"), false, false);
	player->destroy();
	delete player; // WARNING!!! OpenGL is NOT THREAD-SAFE!!!

	if (!connectOnStart) {
		GameObject::SceneRoot.addComponent(new MainMenu());
	}

	return true; // This object is added to SceneRoot, and modifies component list of SceneRoot, so iter is invalidated
}