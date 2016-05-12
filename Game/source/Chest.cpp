#include "Chest.h"
#include <iostream>
#include "Timer.h"

Chest::Chest()
{
}

Chest::Chest(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	int targetID = std::stoi(tokens[1]);
	int threshold = std::stoi(tokens[2]);

	setThreshold(threshold);
	(*idToTarget)[targetID] = this;

}

Chest::~Chest()
{
}

void Chest::create()
{
	initPosition = gameObject->transform.children[0]->getPosition();
}

void Chest::fixedUpdate()
{
	if (isActivated()) {
		std::cout << "chest is opened!" << std::endl;
		float deltaTime = Timer::fixedTimestep;

		openness += (deltaTime) * (isActivated() ? 1 : -1);
		openness = std::min(1.0f, openness);
		openness = std::max(0.0f, openness);

		gameObject->transform.children[0]->setPosition(initPosition + glm::vec3(0, 0, 1) * openness * 2.5f);
	}
}
