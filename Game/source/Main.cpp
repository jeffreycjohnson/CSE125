#include "GameObject.h"
#include "Camera.h"
#include "Renderer.h"

extern void RunEngine();
extern void InitializeEngine();

int main(int argc, char** argv)
{
    InitializeEngine();
    GameObject::SceneRoot.addComponent(Renderer::mainCamera);
    RunEngine();
}