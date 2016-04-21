#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "FPSMovement.h"
#include "RenderPass.h"
#include "Skybox.h"
#include "ActivatorRegistrator.h"

#include <iostream>
#include "ClientManager.h"

extern void RunEngine(int caller);
extern void InitializeEngine(std::string windowName);

int main(int argc, char** argv)
{
    InitializeEngine("CLIENT");

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
	scene->transform.setPosition(0, -1, 0);
	GameObject::SceneRoot.addChild(scene);

	// setup network
	auto clientIDs = ClientManager::initialize("127.0.0.1", "9876");
	for (auto clientID : clientIDs)
	{
		GameObject *player = loadScene("assets/ball.dae");
		player->setName(std::string("player_") + std::to_string(clientID));

		if (clientID == ClientManager::myClientID)
			player->addComponent(Renderer::mainCamera);

		GameObject::SceneRoot.addChild(player);
	}

    RunEngine(0); // running engine as client
}