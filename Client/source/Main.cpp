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
#include "ClientStartingScreen.h"
#include "MainMenu.h"
#include "Crosshair.h"
#include "ClientSidePredictor.h"
#include "Material.h"
#include <iostream>
#include <stdexcept>

extern void RunEngine(NetworkState caller);
extern void InitializeEngine(std::string windowName);

int main(int argc, char** argv)
{
    InitializeEngine("CLIENT");

	GameObject::SceneRoot.addComponent(Renderer::mainCamera);
	GameObject::SceneRoot.addComponent(new ClientStartingScreen("assets/loading.png", ""));

	Input::hideCursor();
	
	auto * r = new Material("assets/TerminalRedUnlocked.mat.ini", false);
	auto * o = new Material("assets/TerminalOrangeUnlocked.mat.ini", false);
	auto * y = new Material("assets/TerminalYellowUnlocked.mat.ini", false);
	auto * g = new Material("assets/TerminalGreenUnlocked.mat.ini", false);
	auto * b = new Material("assets/TerminalBlueUnlocked.mat.ini", false);
	auto * v = new Material("assets/TerminalVioletUnlocked.mat.ini", false);

	// gotta get that CLIENT SIDE PREDICTION
	auto callback = [](GameObject *go) 
	{
		Sensitivity sens(9.0f, 8.5);
		// go->addComponent(new ClientSidePredictor(sens));
	};

	Camera::RegisterCameraAssignmentCallback(callback);

	try
	{
		RunEngine(NetworkState::CLIENT_MODE); // let the loading screen code set up the networking
	}
	catch (...)
	{
		const auto& eptr = std::current_exception();
	}

}
