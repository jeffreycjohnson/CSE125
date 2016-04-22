#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "FPSMovement.h"
#include "RenderPass.h"
#include "Skybox.h"
#include "ActivatorRegistrator.h"

#include <iostream>
#include "ServerManager.h"

extern void RunEngine(int caller);
extern void InitializeEngine(std::string windowName);

int main(int argc, char** argv)
{
    InitializeEngine("SERVER");

	for (auto& skybox : Renderer::mainCamera->passes)
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
	}
	
	GameObject *scene = loadScene("assets/artsy.dae");
	scene->setID(101);
	scene->transform.setPosition(0, -1, 0);
	GameObject::SceneRoot.addChild(scene);
	GameObject::SceneRoot.addComponent(Renderer::mainCamera);
	GameObject::SceneRoot.setID(102);
	auto clientIDs = ServerManager::initialize("9876", 1);
	for (auto clientID : clientIDs)
	{
		GameObject *player = loadScene("assets/ball.dae");
		player->addComponent(new FPSMovement(clientID, 1.5f, .25f, glm::vec3(clientID, .25f, clientID), glm::vec3(0, 1, 0)));
		player->setID(103);
		player->setName(std::string("player_") + std::to_string(clientID));
		GameObject::SceneRoot.addChild(player);
	}
		
    RunEngine(1); // Run engine as server
}