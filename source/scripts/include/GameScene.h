#ifndef INCLUDE_GAMESCENE_H
#define INCLUDE_GAMESCENE_H

#include "GameObject.h"

class Scene {
public:
	virtual void loop() = 0;
};

class GameScene : public Scene
{
public:
	GameObject* sun;

	GameScene();
	~GameScene();
	void loop() override;
};

#endif
