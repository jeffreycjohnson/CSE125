// SERVER MAIN

#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "FPSMovement.h"
#include "RenderPass.h"
#include "Skybox.h"
#include "ActivatorRegistrator.h"
#include "BoxCollider.h"
#include "Config.h"
#include "GodSummoner.h"
#include "Inventory.h"
#include "Sound.h"

#include <iostream>
#include "NetworkManager.h"

extern void RunEngine(NetworkState caller);
extern void InitializeEngine(std::string windowName);

int main(int argc, char** argv)
{
    InitializeEngine("SERVER");

	ConfigFile file("config/options.ini");
	std::string port = file.getString("NetworkOptions", "port");
	int numberOfClients = file.getInt("NetworkOptions", "numclients");

	auto clientIDs = NetworkManager::InitializeServer(port, numberOfClients);
	
	GameObject *scene = loadScene(file.getString("GameSettings","level"));
	scene->transform.setPosition(0, -0.3f, 0);
	GameObject::SceneRoot.addChild(scene);

	bool didSetCamera = false;
	for (auto client : clientIDs) 
	{
		// load the player scene
		GameObject *player = loadScene(file.getString("GameSettings", "player"));

		Sensitivity sens(file.getFloat("GameSettings", "mouseSensitivity"), file.getFloat("GameSettings", "joystickSensitivity"));
		
		// see if we can find a designated spawn point, otherwise make one up
		glm::vec3 spawnPosition = glm::vec3(client, 3, -client);
		std::vector<GameObject*> realSpawn = GameObject::FindAllByPrefix(std::string("spawn_") + std::to_string(client));
		if (realSpawn.size() > 0)
		{
			spawnPosition = realSpawn[0]->transform.getWorldPosition();
		}

		// create an object used to house the camera
		GameObject *verticality = new GameObject;

		// fully init player
		player->addComponent(new FPSMovement(client, sens, spawnPosition, glm::vec3(0, 1, 0), verticality));
		player->addComponent(new Inventory());

		//TODO!!!! REMOVE THIS!!!!!!!!!
		player->addComponent(new Sound("mariojump", false, false, 1.0, true));
		if (client == 0) verticality->addComponent(Renderer::mainCamera);

		NetworkManager::attachCameraTo(client, verticality->getID());
		GameObject::SceneRoot.addChild(player);
	}

	// register and activate the activator registrator
	// the AR is created as a component just to take advantage of `create()`
	ActivatorRegistrator ar;
	GameObject::SceneRoot.addComponent(&ar);

	try
	{
		RunEngine(NetworkState::SERVER_MODE); // running engine as server
	}
	catch (...)
	{
		const auto& eptr = std::current_exception();
	}
}
