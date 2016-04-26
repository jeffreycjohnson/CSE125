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
#include "GodSummoner.h"

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
	
	GameObject *scene = loadScene("assets/pressure.dae");
	scene->transform.setPosition(0, -0.5f, 0);
	GameObject::SceneRoot.addChild(scene);

	// setup network
	GameObject *player = loadScene("assets/cubeman.dae");
	player->addComponent(new FPSMovement(1.5f, 0.5f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
	player->addComponent(Renderer::mainCamera);
	GameObject::SceneRoot.addChild(player);

	// find the collider
	GameObject *box = player->transform.children[1]->children[0]->children[0]->gameObject;
	GameObject *monkey = GameObject::FindByName("Suzanne");
	if (box != nullptr && monkey != nullptr)
	{
		std::cout << "Hey I found the boxCollider!!" << std::endl;
		box->addComponent(new GodSummoner(monkey));
	}

    RunEngine(2); // running engine as client
}