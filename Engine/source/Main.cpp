#include "ForwardDecs.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"
#include "Sound.h"
#include "ObjectLoader.h"
#include "ThreadPool.h"
#include "Camera.h"
#include "OctreeManager.h"
#include "Config.h"
#include "BoxCollider.h"
#include "NetworkManager.h"

#include <glfw3.h>
#include <chrono>

GLFWwindow * mainWindow;

void LoadDebugOptions(ConfigFile&);
void LoadOctreeOptionsAndInitialize(ConfigFile&);

void InitializeEngine(std::string windowName)
{
    workerPool = new ThreadPool();

    Timer::init(2.5f);
    if (!glfwInit())
    {
        LOG("GLFW failed to initialize");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    int width = 1024;
    int height = 768;

	// Load from "config/options.ini"
	ConfigFile file("config/options.ini");
	width  = file.getInt("GraphicsOptions", "width");
	height = file.getInt("GraphicsOptions", "height");

	LoadDebugOptions(file);
	LoadOctreeOptionsAndInitialize(file);

    //zconst GLFWvidmode* mode = glfwGetVideoMode(monitor);
	GLFWwindow* window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

    //set callbacks
    glfwSetWindowFocusCallback(window, Renderer::focus);

    if (!window)
    {
        LOG("Window creation failed");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    mainWindow = window;

    glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(1);
	Sound::init();
	Renderer::init(width, height);
    Input::init(window);
	Input::setCursor("assets/cursor/cursor.png", 32, 32);
    //mouse events
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetScrollCallback(window, Input::scroll_callback);

    // Loads mesh data for primatives, but we don't need it in a GameObject
	delete loadScene("assets/Primatives.obj");
}

void InitializeEngine() { InitializeEngine("CSE 125"); }

// ConfigFile is a local variable in InitializeEngine, but we will likely be adding
// lots more to this section, so I split it off from the main function.
void LoadDebugOptions(ConfigFile& file) {

	Renderer::drawDebug = file.getBool("DebugOptions", "debugEnabledOnStart");
	DebugPass::drawColliders = file.getBool("DebugOptions", "drawColliders");
	DebugPass::drawLights = file.getBool("DebugOptions", "drawLights");
	DebugPass::drawDynamicOctree = file.getBool("DebugOptions", "drawDynamicOctree");
	DebugPass::drawStaticOctree = file.getBool("DebugOptions", "drawStaticOctree");
	DebugPass::colliderColor = file.getColor("DebugOptions", "colliderColor");
	DebugPass::collidingColor = file.getColor("DebugOptions", "collidingColor");
	DebugPass::octreeColor = file.getColor("DebugOptions", "octreeColor");

	BoxCollider::drawBoxPoints = file.getBool("DebugOptions", "drawBoxPoints");

}

void LoadOctreeOptionsAndInitialize(ConfigFile& file) {

	OctreeManager* manager = new OctreeManager();
	GameObject::SceneRoot.addComponent(manager);

	glm::vec3 min, max;
	min.x = file.getFloat("OctreeOptions", "xmin");
	max.x = file.getFloat("OctreeOptions", "xmax");
	min.y = file.getFloat("OctreeOptions", "ymin");
	max.y = file.getFloat("OctreeOptions", "ymax");
	min.z = file.getFloat("OctreeOptions", "zmin");
	max.z = file.getFloat("OctreeOptions", "zmax");

	//OctreeManager::useNaive = file.getBool("OctreeOptions", "useNaive");
	
	// Build the octrees; although no objects have been loaded yet
	manager->buildStaticOctree(min, max);
	manager->buildDynamicOctree(min, max);

	// TODO: When this is merged into develop, register the before 
	// and after callbacks on the OctreeManager.
}

// Caller will be 0 if client, 1 if server, 2 if modelviewer.
void RunEngine(NetworkState caller)
{
	NetworkManager::setState(caller);
	std::function<void()> updateScene = std::bind(GameObject::UpdateScene, caller);
    auto update = workerPool->createJob(updateScene)->queue();
    workerPool->wait(update);
	bool debugToggle = Renderer::drawDebug;

	while (!glfwWindowShouldClose(mainWindow))
	{
        Timer::update();
        Input::update();

		/**
		 * Note: Client/server managing is done within GameObject::UpdateScene
		 *       to guarantee synchronization with fixed tick rate or variable tick rate
		 */

		workerPool->createJob(Sound::updateFMOD)->queue();
		if (Input::getKeyDown("escape")) {
			Renderer::drawDebug = !Renderer::drawDebug;
		}
		Renderer::loop(caller);
	}

	Renderer::shutdown = true;
    delete workerPool;
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
