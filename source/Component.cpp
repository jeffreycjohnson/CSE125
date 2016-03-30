#include "Component.h"

void Component::update(float deltaTime){}
void Component::draw() {}
void Component::setGameObject(GameObject* go) {
    gameObject = go;
}
void Component::debugDraw() {}

void Component::collisionEnter(GameObject* other)
{
}

void Component::create()
{
}

void Component::destroy()
{
}

/*void Component::collisionExit(GameObject* other)
{
}

void Component::collisionStay(GameObject* other)
{
}

void Component::activate()
{
}

void Component::deactivate()
{
}*/

void Component::fixedUpdate()
{
}
