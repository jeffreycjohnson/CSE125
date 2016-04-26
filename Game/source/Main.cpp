// SERVER MAIN

#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "FPSMovement.h"
#include "RenderPass.h"
#include "Skybox.h"
#include "ActivatorRegistrator.h"

#include <iostream>
#include "NetworkManager.h"

extern void RunEngine(int caller);
extern void InitializeEngine(std::string windowName);

int main(int argc, char** argv)
{
	InitializeEngine("SERVER");
	auto clientIDs = NetworkManager::InitializeServer("9876", 1);

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

	GameObject::SceneRoot.addChild(scene);
	scene->transform.setPosition(0, -1, 0);

	GameObject::SceneRoot.addComponent(Renderer::mainCamera);
	Renderer::mainCamera->fov = glm::radians(90.0f);
		
	try
	{
		RunEngine(1); // running engine as server
	}
	catch (...)
	{
		const auto& eptr = std::current_exception();
	}
}