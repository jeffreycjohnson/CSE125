#include "GameObject.h"
#include "Mesh.h"
#include "GPUEmitter.h"
#include "Light.h"
#include "ParticleTrail.h"
#include "Timer.h"
#include "Renderer.h"
#include "Material.h"
#include "Networkmanager.h"
#include "NetworkUtility.h"
#include "ObjectLoader.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>

std::vector<void(*)(void)> GameObject::preFixedCallbacks;
std::vector<void(*)(void)> GameObject::postFixedCallbacks;
std::vector<void(*)(void)> GameObject::preVarCallbacks;
std::vector<void(*)(void)> GameObject::postVarCallbacks;

int GameObject::objectIDCounter;
std::multimap<std::string, GameObject*> GameObject::nameMap;
std::map<int, GameObject*> GameObject::idMap;
GameObject GameObject::SceneRoot;

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
	this->setID(GameObject::objectIDCounter++);
}

GameObject::GameObject(int id) {
	transform.setGameObject(this);
	dead = false;
	active = true;
	visible = true;
	newlyCreated = true;
	this->setID(id);
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
	go->transform.setParent(&transform);

	go->postToNetwork();
}

void GameObject::removeChild(GameObject * go)
{
	auto child = std::find(transform.children.begin(), transform.children.end(), &go->transform);
	if (child != transform.children.end())
	{
		throw std::runtime_error("Cannot remove child from gameobject parent if child is not child");
	}

	transform.children.erase(child);
	go->transform.setParent(nullptr);
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
    auto parent = transform.getParent();
    while(parent)
    {
        if (parent->gameObject == go) return true;
        parent = parent->getParent();
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
	// don't fraget transform!!
	if (newlyCreated || transform.newlyCreated)
	{
		transform.create();
		transform.newlyCreated = false;
	}

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
	int oldID = this->ID;

	this->ID = -1;
	if (idMap.size() <= 0) return;

	idMap.erase(oldID);
}

void GameObject::postToNetwork()
{
	if (NetworkManager::getState() != SERVER_MODE) return;

	// is this a correct assumption to make??
	if (transform.getParent() == nullptr && getID() != 0)
	{
		// if we don't have a parent
		// and we're not the root
		// then who cares about our data

		return;
	}

	NetworkManager::PostMessage(serialize(), CREATE_OBJECT_NETWORK_DATA, getID());
}

void GameObject::DestroyObjectByID(int objectID) {
	GameObject * obj = SceneRoot.FindByID(objectID);
	if (obj != NULL) 
	{
		obj->destroy();
		std::cout << "Destroyed object " << obj->getID() << std::endl;
	}
	else
	{
		std::cerr << "Tried to destroy non-existant object ID " << objectID << std::endl;
	}
}

void GameObject::Dispatch(const std::vector<char> &bytes, int messageType, int messageID)
{
	if (messageType == CREATE_OBJECT_NETWORK_DATA)
	{
		GameObject::deserializeAndCreate(bytes);
	}
	else if (messageType == DESTROY_OBJECT_NETWORK_DATA)
	{
		GameObject *toDestroy = GameObject::FindByID(messageID);
		if (toDestroy == nullptr)
		{
			throw std::runtime_error("Cannot destroy gameobject that does not exist");
		}

		toDestroy->destroy();
	}
}

std::vector<char> GameObject::serialize()
{
	int myID = getID();

	CreateObjectNetworkData cond(myID);

	std::vector<char> myCreation;
	myCreation.resize(sizeof(cond), 0);
	memcpy(myCreation.data(), &cond, sizeof(cond));

	return myCreation;
}

bool GameObject::deserializeAndCreate(std::vector<char> bytes)
{
	CreateObjectNetworkData cond = structFromBytes<CreateObjectNetworkData>(bytes);
	if (GameObject::FindByID(cond.objectID) != nullptr)
	{
		std::cerr << "Cannot create object with ID " << cond.objectID << ", object with ID already exists" << std::endl;
		return false;
	}

	new GameObject(cond.objectID);
	return true;
}
