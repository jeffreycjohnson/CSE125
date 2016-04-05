#include "ForwardDecs.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"
#include "Sound.h"
#include <glfw3.h>
#include "ObjectLoader.h"
#include "ThreadPool.h"
#include "Camera.h"
#include <chrono>

GLFWwindow * mainWindow;

void InitializeEngine()
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

    //zconst GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(width, height, "CSE 125", nullptr, nullptr);

    //set callbacks
    //glfwSetWindowFocusCallback(window, window_focus_callback);
    //void window_focus_callback(GLFWwindow* window, int focused)

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
    glfwSwapInterval(0);
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

void RunEngine()
{
    auto update = workerPool->createJob(GameObject::UpdateScene)->queue();
    workerPool->wait(update);

	while (!glfwWindowShouldClose(mainWindow))
	{
        Timer::update();
        Input::update();
        workerPool->createJob(Sound::updateFMOD)->queue();
        Renderer::drawDebug = Input::getKey("escape");
        Renderer::loop();
	}

	for (auto camera : Renderer::cameras) {
		delete camera;
	}
    delete workerPool;
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
