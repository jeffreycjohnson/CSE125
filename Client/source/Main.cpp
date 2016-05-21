// CLIENT MAIN

#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "RenderPass.h"
#include "Skybox.h"
#include "ActivatorRegistrator.h"
#include "NetworkManager.h"
#include "Config.h"
#include "Input.h"
#include "MainMenu.h"
#include "Crosshair.h"

#include <iostream>
#include <stdexcept>

extern void RunEngine(NetworkState caller);
extern void InitializeEngine(std::string windowName);

int main(int argc, char** argv)
{
    InitializeEngine("CLIENT");
	ConfigFile file("config/options.ini");

	std::string serverip = file.getString("NetworkOptions", "serverip");
	std::string port = file.getString("NetworkOptions", "port");
	bool connectOnStart = file.getBool("GameSettings", "skipMenu");

	if (connectOnStart) {
		auto pair = NetworkManager::InitializeClient(serverip, port);
	}
	else {
		NetworkManager::InitializeOffline();
	}

	// cache all meshes
	GameObject *scene = loadScene(file.getString("GameSettings", "level"));
	scene->destroy();
	delete scene;

	GameObject *player = loadScene(file.getString("GameSettings", "player"));
	player->destroy();
	delete player;

	GameObject::SceneRoot.addComponent(Renderer::mainCamera);
	GameObject::SceneRoot.addComponent(new Crosshair(file.getString("GameSettings", "crosshairSprite")));
	if (!connectOnStart) {
		GameObject::SceneRoot.addComponent(new MainMenu());
	}

	Input::hideCursor();


	try
	{
		RunEngine(NetworkState::CLIENT_MODE); // running engine as client
	}
	catch (...)
	{
		const auto& eptr = std::current_exception();
	}
}
