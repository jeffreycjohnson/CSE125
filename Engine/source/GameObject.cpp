#include "GameObject.h"
#include "Mesh.h"
#include "GPUEmitter.h"
#include "Light.h"
#include "ParticleTrail.h"
#include "Timer.h"
#include "Renderer.h"
#include "Material.h"
#include "ObjectLoader.h"
#include <iterator>
#include <iostream>

std::vector<void(*)(void)> GameObject::preFixedCallbacks;
std::vector<void(*)(void)> GameObject::postFixedCallbacks;
std::vector<void(*)(void)> GameObject::preVarCallbacks;
std::vector<void(*)(void)> GameObject::postVarCallbacks;

GameObject GameObject::SceneRoot;
std::multimap<std::string, GameObject*> GameObject::nameMap;
std::multimap<int, GameObject*> GameObject::idMap;
int GameObject::objectIDCounter;

GameObject * GameObject::FindByName(const std::string& name)
{
    auto iter = nameMap.find(name);
    return iter == nameMap.end() ? nullptr : iter->second;
}


GameObject * GameObject::FindByID(const int& id)
{
	auto iter = idMap.find(id);
	return iter == idMap.end() ? nullptr : iter->second;
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
	while (Timer::nextFixedStep()) {
		for (auto& callback : preFixedCallbacks) callback();

		// server or offline
		SceneRoot.fixedUpdate();

		for (auto& callback : postFixedCallbacks) callback();
	}

	// ONLY client and offline get variable update
	if (caller == 0 || caller == 2)  
	{
		for (auto& callback : preVarCallbacks) callback();

		// client or offline
		SceneRoot.update((float)Timer::deltaTime());

		for (auto& callback : postVarCallbacks) callback();

	}
}

void GameObject::AddPreFixedUpdateCallback(void(*callback)(void))
{
	GameObject::preFixedCallbacks.push_back(callback);
}

void GameObject::AddPostFixedUpdateCallback(void(*callback)(void))
{
	GameObject::postFixedCallbacks.push_back(callback);
}

void GameObject::AddPreUpdateCallback(void(*callback)(void))
{
	GameObject::preVarCallbacks.push_back(callback);
}

void GameObject::AddPostUpdateCallback(void(*callback)(void))
{
	GameObject::postVarCallbacks.push_back(callback);
}

GameObject::GameObject() {
	transform.setGameObject(this);
	dead = false;
    active = true;
	visible = true;
    newlyCreated = true;
	ID = GameObject::objectIDCounter++;
}

GameObject::GameObject(int id) {
	transform.setGameObject(this);
	dead = false;
	active = true;
	visible = true;
	newlyCreated = true;
	ID = id;
}

GameObject::~GameObject() {
	for (auto child : transform.children) {
		if(child && child->gameObject) delete child->gameObject;
	}
	for (auto component : componentList) {
		if(component) delete component;
	}
    removeName();
	removeID();
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

// TODO wait why is ID a multimap?? 
void GameObject::setID(const int ID)
{
	removeID();

	this->ID = ID;
	idMap.insert(std::make_pair(ID, this));
}

int GameObject::getID() const
{
	return ID;
}

void GameObject::removeID()
{
	this->ID = -1;

	auto range = idMap.equal_range(this->ID);
	while (range.first != range.second)
	{
		if (range.first->second == this) {
			idMap.erase(range.first);
			return;
		}
		++range.first;
	}
}

int GameObject::createObject() {
	return createObject(GameObject::objectIDCounter++);
}

int GameObject::createObject(int id) {
	GameObject * g = loadScene("assets/ball.dae");
	g->setID(id);
	SceneRoot.addChild(g);
	return g->getID();
}

void GameObject::destroyObjectByID(int objectID) {
	GameObject * obj = SceneRoot.FindByID(objectID);
	if (obj != NULL) {
		obj->destroy();
		std::cout << "Destroyed object " << obj->getID() << std::endl;
	}
}

std::vector<std::vector<char>> GameObject::serializeCreation(int parentID)
{
	std::vector<std::vector<char>> allMessages;

	int myID = getID();
	std::string meshName;

	Mesh* possibleMesh = this->getComponent<Mesh>();
	if (possibleMesh != nullptr)
	{
		meshName = possibleMesh->name;
	}

	TransformNetworkData tnd = transform.serializeAsStruct();
	CreateObjectNetworkData cond(myID, parentID, meshName, tnd);

	std::vector<char> myCreation;
	myCreation.resize(sizeof(cond), 0);
	memcpy(myCreation.data(), &cond, sizeof(cond));

	for (auto& childTransform : transform.children)
	{
		auto& child = childTransform->gameObject;
		std::vector<std::vector<char>> childMessages = child->serializeCreation(myID);

		// extend allMessages with the creation messages of my children
		allMessages.insert(allMessages.end(),
			std::make_move_iterator(childMessages.begin()),
			std::make_move_iterator(childMessages.end()));
	}

	return allMessages;
}