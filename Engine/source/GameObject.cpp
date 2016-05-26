#include "GameObject.h"
#include "Mesh.h"
#include "GPUEmitter.h"
#include "Light.h"
#include "ParticleTrail.h"
#include "Timer.h"
#include "Renderer.h"
#include "Material.h"
#include "NetworkManager.h"
#include "NetworkUtility.h"
#include "ObjectLoader.h"
#include "OctreeManager.h"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>

std::vector<void(*)(void)> GameObject::preFixedCallbacks;
std::vector<void(*)(void)> GameObject::postFixedCallbacks;
std::vector<void(*)(void)> GameObject::preVarCallbacks;
std::vector<void(*)(void)> GameObject::postVarCallbacks;

int GameObject::objectIDCounter = 1;
std::multimap<std::string, GameObject*> GameObject::nameMap;
std::map<int, GameObject*> GameObject::idMap;
GameObject GameObject::SceneRoot(0, "SceneRoot");

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

std::vector<GameObject*> GameObject::FindAllByPrefix(const std::string & name)
{
	std::vector<GameObject*> ret;
	for (auto& group : nameMap)
	{
		if (name.size() > group.first.size()) continue;

		bool isPrefix = std::equal(
			group.first.begin(),
			group.first.begin() + name.size(),
			name.begin()
		);
		if (isPrefix) ret.push_back(group.second);
	}

	return ret;
}

void GameObject::UpdateScene(NetworkState caller)
{
	while (Timer::nextFixedStep()) {
		for (auto& callback : preFixedCallbacks) callback();

		// server or offline
		SceneRoot.fixedUpdate();

		for (auto& callback : postFixedCallbacks) callback();
	}

	// ONLY client and offline get variable update
	if (caller == NetworkState::CLIENT_MODE || /*caller == NetworkState::SERVER_MODE ||*/ caller == NetworkState::OFFLINE)  
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

GameObject::GameObject(int id, std::string name) {
	transform.setGameObject(this);
	dead = false;
	active = true;
	visible = true;
	newlyCreated = true;
	this->setID(id);
	this->name = name;
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

	// Add gameobject to octree
	OctreeManager* ptr = GameObject::SceneRoot.getComponent<OctreeManager>();
	if (ptr != nullptr) {
		ptr->insertGameObject(go);
	}
	
	go->postToNetwork();
}

void GameObject::removeChild(GameObject * go)
{
	auto child = std::find(transform.children.begin(), transform.children.end(), &go->transform);
	if (child == transform.children.end())
	{
		FATAL("Cannot remove child from gameobject parent if child is not child");
	}

	transform.children.erase(child);
	go->transform.setParent(nullptr);
	NetworkManager::PostMessage(go->serialize(), DESTROY_OBJECT_NETWORK_DATA, go->getID());
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

GameObject* GameObject::findChildByNameContains(const std::string& name)
{
	for (auto child : transform.children)
	{
		if (child->gameObject->name.find(name) != std::string::npos) {
			return child->gameObject;
		}
	}
	return nullptr;
}

void GameObject::drawUI() {
	// TODO: A similar hack could be implemented in fixedUpdate/update/debugDraw,
	//   but I think this will require further discussion. Generally displeased
	//   with having to return a bool. Will try making this function a little smarter
	//   later, but for now it works as intended.
	if (visible && active && !dead) {
		for (auto component : componentList) {
			if (component->visible && component->active)
				if (component->drawUI()) {
					break; // Iterator invalidated
				}
		}
		for (auto child : transform.children) {
			(child->gameObject)->drawUI();
		}
	}
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
			if (mesh->getMaterial() && mesh->getMaterial()->transparent) {
				Renderer::renderBuffer.forward.push_back(mesh);
			}
			else if(mesh->getMaterial())
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

    auto range = nameMap.equal_range(name); // TODO: ModelViewer crashes here if you close model window & not console window
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
			FATAL("Cannot destroy gameobject that does not exist");
		}

		toDestroy->destroy();
	}
}

std::vector<char> GameObject::serialize()
{
	int myID = getID();

	CreateObjectNetworkData cond(myID, name, visible, active);
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
		GameObject * go = GameObject::FindByID(cond.objectID);
		std::cout << "Updating object with id " << go->getID() << " and name " << go->getName() << std::endl;

		go->active = cond.active;
		go->visible = cond.visible;
	}
	else {
		std::cout << "Creating object with id " << cond.objectID << " and name " << cond.name << std::endl;
		new GameObject(cond.objectID, cond.name);
	}
	return true;
}

void GameObject::setVisible(bool visible)
{
	this->visible = visible;
	postToNetwork();
}

void GameObject::setActive(bool active, ActiveChildren flag) {
	this->active = active;

	if (flag != OnlySetParent &&
		!(flag == SetChildrenIfInactive && active))
	{
		for (auto child : transform.children) {
			child->gameObject->setActive(active, flag);
		}
	}

	postToNetwork();
}

bool GameObject::getVisible()
{
	return visible;
}

bool GameObject::getActive()
{
	return active;
}
