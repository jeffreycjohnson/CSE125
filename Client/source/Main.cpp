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
	auto pair = NetworkManager::InitializeClient(serverip, port);
	//NetworkManager::InitializeOffline();

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

	// cache all meshes
	GameObject *scene = loadScene("assets/key_action.dae");
	scene->destroy();
	delete scene;

	GameObject *player = loadScene("assets/CorridorPuzzle.dae");
	player->destroy();
	delete player;

	GameObject::SceneRoot.addComponent(Renderer::mainCamera);

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
