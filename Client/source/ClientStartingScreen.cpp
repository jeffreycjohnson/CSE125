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

	std::cerr << "Did we get into there?" << std::endl;
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

	GameObject::SceneRoot.addComponent(new Crosshair(file.getString("GameSettings", "crosshairSprite")));
	if (!connectOnStart) {
		GameObject::SceneRoot.addComponent(new MainMenu());
	}

	std::cerr << "LOADING COMPLETE" << std::endl;

	return true; // This object is added to SceneRoot, and modifies component list of SceneRoot, so iter is invalidated
}