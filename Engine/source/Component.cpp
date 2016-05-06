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
}

void Component::collisionExit(GameObject* other)
{
}

void Component::collisionStay(GameObject* other)
{
}

void Component::staticCollisionEnter(GameObject* other)
{
}

void Component::staticCollisionStay(GameObject* other)
{
}

void Component::staticCollisionExit(GameObject* other)
{
}

/*
void Component::activate()
{
}

void Component::deactivate()
{
}*/

void Component::fixedUpdate()
{
}

// serialization
std::vector<char> Component::serialize() { return std::vector<char>(); };
void Component::deserializeAndApply(std::vector<char> bytes) {};