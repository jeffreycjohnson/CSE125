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

extern void RunEngine();
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
	player->addComponent(new FPSMovement(1.5f, .25f, glm::vec3(0,0,0), glm::vec3(0,1,0)));
	player->addComponent(Renderer::mainCamera);

	GameObject::SceneRoot.addChild(player);

	GameObject *scene = loadScene("assets/artsy.dae");
	//auto* ar = new ActivatorRegistrator;
	//scene->addComponent(ar);
	scene->transform.setPosition(0, -1, 0);

	GameObject::SceneRoot.addChild(scene);

    RunEngine();
}