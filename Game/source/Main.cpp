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

	/*for (auto& skybox : Renderer::mainCamera->passes)
	{
		SkyboxPass* sp = dynamic_cast<SkyboxPass*>(skybox.get());
		if (sp != nullptr)
		{
			std::string imgs[] = {
				"assets/skyboxes/icyhell/icyhell_lf.tga",
				"assets/skyboxes/icyhell/icyhell_rt.tga",
				"assets/skyboxes/icyhell/icyhell_up.tga",
				"assets/skyboxes/icyhell/icyhell_dn.tga",
				"assets/skyboxes/icyhell/icyhell_ft.tga",
				"assets/skyboxes/icyhell/icyhell_bk.tga",
};
			sp->skybox = new Skybox(imgs);

			break;
		}
	}*/
	
	GameObject *scene = loadScene(file.getString("GameSettings","level"));
	scene->transform.setPosition(0, -0.3f, 0);
	GameObject::SceneRoot.addChild(scene);

	bool didSetCamera = false;
	for (auto client : clientIDs) {
		GameObject *player = loadScene(file.getString("GameSettings", "player"));
		GameObject *verticality = new GameObject;

		player->addComponent(new FPSMovement(client, 4.0f, 0.5f, glm::vec3(client * 2, 5, -client * 2), glm::vec3(0, 1, 0), verticality));
		player->addComponent(new Inventory());
		player->addComponent(new Sound("mariojump", false, false, 1.0, false));

		if (client == 0)
		{
			verticality->addComponent(Renderer::mainCamera);
		}

		NetworkManager::attachCameraTo(client, verticality->getID());
		GameObject::SceneRoot.addChild(player);
	}

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
