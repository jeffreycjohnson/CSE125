#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"
#include "Input.h"
#include "Collision.h"
#include <iostream>

extern void RunEngine(int caller);
extern void InitializeEngine();

// provides camera controls
class Controls : public Component {
public:
	void update(float dt) override {
		// we really should redo input...
		glm::quat roll = glm::angleAxis(Input::getAxis("roll") * dt * 2, glm::vec3(0, 0, -1));
		glm::quat pitch = glm::angleAxis(Input::getAxis("yaw") * dt * 2, glm::vec3(0, -1, 0));
		glm::quat yaw = glm::angleAxis(Input::getAxis("pitch") * dt * 2, glm::vec3(-1, 0, 0));
		//gameObject->transform.rotate(roll*pitch*yaw);
		Renderer::mainCamera->gameObject->transform.translate(glm::vec3(Input::getAxis("roll") * dt * 2, Input::getAxis("yaw") * dt * 2, Input::getAxis("pitch") * 2 * dt));
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

	// Octree stuff
	Octree::DYNAMIC_TREE = new Octree(glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10));
	Octree::DYNAMIC_TREE->build(Octree::BOTH); // Include all objs for now
	
	RunEngine(2); // Run Engine as modelviewer
	
	delete Octree::DYNAMIC_TREE;
	Octree::DYNAMIC_TREE = nullptr;
}
