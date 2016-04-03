#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"
#include "ObjectLoader.h"

extern void RunEngine();
extern void InitializeEngine();

int main(int argc, char** argv)
{
    InitializeEngine();
    GameObject::SceneRoot.addComponent(Renderer::mainCamera);
    RunEngine();
}