#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "PlayerMovement.h"

extern void RunEngine();
extern void InitializeEngine();

int main(int argc, char** argv)
{
    InitializeEngine();

	/*
	Skyboxes

	1. Renderer::mainCamera -> passes
	2. dynamic_cast for SkyboxPass element in list
	3. skyboxpass->skybox = new Skybox
	*/


	GameObject *ballman = loadScene("assets/ballman.dae");
	ballman->transform.setPosition(0, 0, -3);
	ballman->addComponent(new PlayerMovement);

	GameObject::SceneRoot.addChild(ballman);
	GameObject::SceneRoot.addComponent(Renderer::mainCamera);
    RunEngine();
}