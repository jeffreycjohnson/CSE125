#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "Input.h"
#include "Collision.h"
#include "GPUEmitter.h"
#include <iostream>
#include "NetworkManager.h"

extern void RunEngine(NetworkState caller);
extern void InitializeEngine();

GPUEmitter* particleSystem = nullptr;

// provides camera controls
class Controls : public Component {
public:
	void update(float dt) override {
		// we really should redo input...
		glm::quat roll = glm::angleAxis(Input::getAxis("roll") * dt * 2, glm::vec3(0, 0, -1));
		glm::quat pitch = glm::angleAxis(Input::getAxis("yaw") * dt * 2, glm::vec3(0, -1, 0));
		glm::quat yaw = glm::angleAxis(Input::getAxis("pitch") * dt * 2, glm::vec3(-1, 0, 0));
		//gameObject->transform.rotate(roll*pitch*yaw);
		float speed = 10.0; // 10 units/sec
		if (particleSystem) {
			//particleSystem->play();
		}
		if (Input::getButtonDown("space")) {
			particleSystem->play();
		}
		Renderer::mainCamera->gameObject->transform.translate(glm::vec3(Input::getAxis("right") * dt * speed, Input::getAxis("forward") * dt * speed, Input::getAxis("pitch") * speed * dt));
	}
};

int main(int argc, char** argv)
{
	std::cout << "Enter Model Filename: ";
	std::string name;
	std::cin >> name;

	InitializeEngine();

	// setup the camera offset from the scene center
	auto camera = new GameObject();
	camera->addComponent(Renderer::mainCamera);
	camera->transform.setPosition(0, 0, 5);
	GameObject::SceneRoot.addChild(camera);

	// load the scene
	auto scene = loadScene(name);
	scene->addComponent(new Controls());
	GameObject::SceneRoot.addChild(scene);

	// Load particle emitter
	auto emitter = GameObject::SceneRoot.FindByName("DefaultEmitter");
	if (emitter != nullptr) {
		particleSystem = emitter->getComponent<GPUEmitter>();
	}

	RunEngine(NetworkState::OFFLINE); // Run Engine as modelviewer
	
}
