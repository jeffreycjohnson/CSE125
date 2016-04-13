#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "FPSMovement.h"
#include "RenderPass.h"
#include "Skybox.h"
#include "ActivatorRegistrator.h"

#include <iostream>
#include "ClientNetwork.h"
#include "ServerLoop.h"

extern void RunEngine(int caller);
extern void InitializeEngine();

int main(int argc, char** argv)
{
    InitializeEngine();

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

	GameObject *player = new GameObject;
	player->setName("player");
	player->addComponent(Renderer::mainCamera);
	GameObject::SceneRoot.addChild(player);

	// add the server component
	ServerLoop* sl = new ServerLoop("9876");
	sl->create();
	GameObject *server = new GameObject;
	server->addComponent(sl);
	GameObject::SceneRoot.addChild(server);

    RunEngine(1); // Run engine as server
}