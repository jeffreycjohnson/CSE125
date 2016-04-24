#include "Component.h"

void Component::update(float deltaTime){}
void Component::draw() {}
void Component::setGameObject(GameObject* go) {
    gameObject = go;
}
void Component::debugDraw() {}

void Component::create()
{
}

void Component::destroy()
{
}

void Component::collisionEnter(GameObject* other)
{
	LOG("Collision Enter!!!1");
}

void Component::collisionExit(GameObject* other)
{
	LOG("Collision Exit!!!1");
}

void Component::collisionStay(GameObject* other)
{
	LOG("Collision Stay!!!1");
}

void Component::staticCollisionEnter(GameObject* other)
{
	LOG("Collision Static ENTER!!!1");
}

void Component::staticCollisionStay(GameObject* other)
{
	LOG("Collision Static STAY!!!1");

}

void Component::staticCollisionExit(GameObject* other)
{
	LOG("Collision Static EXIt!!!1");

}

/*
void Component::activate()
{
}

void Component::deactivate()
{
}*/

void Component::beforeFixedUpdate()
{
}

void Component::fixedUpdate()
{
}

void Component::afterFixedUpdate()
{
}

// serialization
std::vector<char> Component::serialize() { return std::vector<char>(); };
void Component::deserializeAndApply(std::vector<char> bytes) {};