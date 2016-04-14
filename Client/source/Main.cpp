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
	player->addComponent(Renderer::mainCamera);
	player->setName("player");
	GameObject::SceneRoot.addChild(player);

	GameObject *scene = loadScene("assets/artsy.dae");
	scene->transform.setPosition(0, -1, 0);
	GameObject::SceneRoot.addChild(scene);

	// setup network
	ClientNetwork::SetupTCPConnection("127.0.0.1", "9876");

    RunEngine(0); // running engine as client
}