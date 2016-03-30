#include "GameScene.h"
#include "BoxCollider.h"
#include "Renderer.h"
#include "Camera.h"

GameScene::GameScene()
{
    GameObject::SceneRoot.addComponent(Renderer::mainCamera);
}

GameScene::~GameScene()
{
}

void GameScene::loop() {
	BoxCollider::updateColliders();
}
