#include "GameObject.h"
#include "Mesh.h"
#include "GPUEmitter.h"
#include "Light.h"
#include "ParticleTrail.h"
#include "Timer.h"
#include "Renderer.h"
#include "Material.h"
#include "ServerManager.h"
#include "ClientManager.h"
#include <iostream>
#include <functional>

GameObject GameObject::SceneRoot;
std::multimap<std::string, GameObject*> GameObject::nameMap;

GameObject * GameObject::FindByName(const std::string& name)
{
    auto iter = nameMap.find(name);
    return iter == nameMap.end() ? nullptr : iter->second;
}

std::vector<GameObject*> GameObject::FindAllByName(const std::string& name)
{
    std::vector<GameObject*> ret;
    auto range = nameMap.equal_range(name);
    while (range.first != range.second) {
        ret.push_back(range.first->second);
        ++range.first;
    }
    return ret;
}

void GameObject::UpdateScene(int caller)
{
	if (caller == 1 || caller == 2) 
	{ 
		while (Timer::nextFixedStep()) {
			if (caller == 1) ServerManager::receiveMessages();

			// server or offline
			SceneRoot.beforeFixedUpdate();
			SceneRoot.fixedUpdate();
			SceneRoot.afterFixedUpdate();

			if (caller == 1) ServerManager::sendMessages();
		}
	}

	if (caller == 0 || caller == 2)  
	{
		if (caller == 0) ClientManager::sendMessages();

		// client or offline
		SceneRoot.update((float)Timer::deltaTime());

		if (caller == 0) ClientManager::receiveMessages();
	}
}

GameObject::GameObject() {
	transform.setGameObject(this);
	dead = false;
    active = true;
	visible = true;
    newlyCreated = true;
}

GameObject::~GameObject() {
	for (auto child : transform.children) {
		if(child && child->gameObject) delete child->gameObject;
	}
	for (auto component : componentList) {
		if(component) delete component;
	}
    removeName();
}

void GameObject::addChild(GameObject* go) {
    transform.children.push_back(&go->transform);
    go->transform.parent = &transform;
}

void GameObject::destroy() {
	dead = true;
    active = false;
    for (auto child : transform.children) {
        child->destroy();
    }
    for (auto component : componentList)
    {
        component->destroy();
        component->active = false;
    }
}

void GameObject::hideAll()
{
	Mesh* mesh = getComponent<Mesh>();
	if (mesh != nullptr)
		mesh->visible = false;

	ParticleTrail* trail = getComponent<ParticleTrail>();
	if (trail != nullptr)
		trail->emitting = false;

	for (auto child : transform.children)
	{
		child->gameObject->hideAll();
	}
}

bool GameObject::isChildOf(GameObject* go) const
{
    auto parent = transform.parent;
    while(parent)
    {
        if (parent->gameObject == go) return true;
        parent = parent->parent;
    }
    return false;
}

GameObject* GameObject::findChildByName(const std::string& name)
{
    for(auto child : transform.children)
    {
        if (child->gameObject->name == name) return child->gameObject;
    }
    return nullptr;
}

void GameObject::debugDraw() {
    if (visible && active && !dead) {
        for (auto component : componentList) {
            if (component->visible && component->active)
                component->debugDraw();
        }
        for (auto child : transform.children) {
            (child->gameObject)->debugDraw();
        }
    }
}

void GameObject::update(float deltaTime)
{
    for (auto component : componentList)
    {
		if (newlyCreated || component->newlyCreated) 
		{
			component->create();
			component->newlyCreated = false;
		}
    }
	newlyCreated = false;

    if (dead || !active) return;
	for (unsigned int i = 0; i < transform.children.size(); i++)
    {
        auto object = transform.children[i];
		if (object->gameObject->dead)
		{
			delete object->gameObject;
			transform.children.erase(transform.children.begin() + i);
		}
		else object->gameObject->update(deltaTime);
    }
    for (auto component : componentList)
    {
        if(component->active) component->update(deltaTime);
    }
}

void GameObject::beforeFixedUpdate()
{
	for (auto component : componentList)
	{
		if (newlyCreated || component->newlyCreated)
		{
			component->create();
			component->newlyCreated = false;
		}
	}
	newlyCreated = false;

	if (dead || !active) return;
	for (unsigned int i = 0; i < transform.children.size(); i++)
	{
		transform.children[i]->gameObject->beforeFixedUpdate();
	}
	for (auto component : componentList)
	{
		if (component->active) component->beforeFixedUpdate();
	}
}

void GameObject::fixedUpdate()
{
	for (auto component : componentList)
	{
		if (newlyCreated || component->newlyCreated)
		{
			component->create();
			component->newlyCreated = false;
		}
	}
	newlyCreated = false;

    if (dead || !active) return;
    for (unsigned int i = 0; i < transform.children.size(); i++)
    {
        transform.children[i]->gameObject->fixedUpdate();
    }
    for (auto component : componentList)
    {
        if (component->active) component->fixedUpdate();
    }
}

void GameObject::afterFixedUpdate()
{
	for (auto component : componentList)
	{
		if (newlyCreated || component->newlyCreated)
		{
			component->create();
			component->newlyCreated = false;
		}
	}
	newlyCreated = false;

	if (dead || !active) return;
	for (unsigned int i = 0; i < transform.children.size(); i++)
	{
		transform.children[i]->gameObject->beforeFixedUpdate();
	}
	for (auto component : componentList)
	{
		if (component->active) component->afterFixedUpdate();
	}
}

void GameObject::extract()
{
	if (visible && active && !dead) {
		Mesh* mesh;
		if ((mesh = getComponent<Mesh>()) != nullptr && mesh->active) {
			if (mesh->material && mesh->material->transparent) {
				Renderer::renderBuffer.forward.push_back(mesh);
			}
			else if(mesh->material)
			{
                Renderer::renderBuffer.deferred.push_back(mesh);
			}
		}
		GPUEmitter* emitter;
		if ((emitter = getComponent<GPUEmitter>()) != nullptr && emitter->active) {
            Renderer::renderBuffer.particle.push_back(emitter);
		}
		ParticleTrail* trail;
		if ((trail = getComponent<ParticleTrail>()) != nullptr && trail->active) {
            Renderer::renderBuffer.particle.push_back(trail);
		}
		Light* light;
		if ((light = getComponent<Light>()) != nullptr && light->active) {
            Renderer::renderBuffer.light.push_back(light);
		}
	}

	for (auto child : transform.children) {
		(child->gameObject)->extract();
	}
}

void GameObject::collisionEnter(GameObject* other)
{
    if (!active || dead) return;
	for (auto component : componentList)
	{
        if (!component->active) continue;
		component->collisionEnter(other);
	}
}

// Okay, I think the Component::* is called the "member function" pointer
// And I totally, 100% take the blame for this black magic
//  -- Dexter
void GameObject::collisionCallback(GameObject* other, void(Component::*callback)(GameObject*)) {
	if (!active || dead) return;
	for (auto component : componentList)
	{
		if (!component->active) continue;
		// Bind the arguments to an std::function, which is a callable object
		auto func = std::bind(callback, component, other);
		func(); // gah, this is so freaking cool, but so... sooo bad
	}
}

/*
void GameObject::collisionStay(GameObject* other)
{
    if (!active || dead) return;
    for (auto component : componentList)
    {
        if (!component->active) continue;
        component->collisionStay(other);
    }
}

void GameObject::collisionExit(GameObject* other)
{
    if (!active || dead) return;
    for (auto component : componentList)
    {
        if (!component->active) continue;
        component->collisionExit(other);
    }
}
*/

void GameObject::setName(const std::string& name)
{
    removeName();

	this->name = name;
    nameMap.insert(std::make_pair(name, this));
}

std::string GameObject::getName() const
{
    return name;
}

void GameObject::removeName()
{
	this->name = "";

    auto range = nameMap.equal_range(name);
    while (range.first != range.second)
    {
        if (range.first->second == this) {
            nameMap.erase(range.first);
            return;
        }
        ++range.first;
    }
}
