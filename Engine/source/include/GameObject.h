#ifndef INCLUDE_GAME_OBJECT_H
#define INCLUDE_GAME_OBJECT_H

#include "ForwardDecs.h"
#include "Transform.h"
#include "Component.h"
#include <vector>
#include <map>

class GameObject
{
private:
	static std::vector<void (*)(void)> preFixedCallbacks;
	static std::vector<void (*)(void)> postFixedCallbacks;
	static std::vector<void (*)(void)> preVarCallbacks;
	static std::vector<void (*)(void)> postVarCallbacks;

public:
	Transform transform;
	bool visible, active;
	static int objectIDCounter;
    static GameObject SceneRoot;
	static GameObject* FindByName(const std::string& name);
	static GameObject* FindByID(const int& id);
	static std::vector<GameObject*> FindAllByName(const std::string& name);
	static void UpdateScene(int caller);

	// registering callbacks for updates
	static void AddPreFixedUpdateCallback(void(*callback)(void));
	static void AddPostFixedUpdateCallback(void(*callback)(void));
	static void AddPreUpdateCallback(void(*callback)(void));
	static void AddPostUpdateCallback(void(*callback)(void));

	static int createObject();
	static int createObject(int id);
	static void destroyObjectByID(int objectID);

	GameObject();
	GameObject(int id);

	~GameObject();

    template<typename T>
    void addComponent(T* c) {
        removeComponent<T>();
        c->setGameObject(this);
        componentList.push_back(c);
    }
    template<typename T>
    bool removeComponent()
    {
        for (auto component = componentList.begin(); component != componentList.end(); ++component)
        {
            T* test = dynamic_cast<T*>(*component);
            if (test)
            {
                test->setGameObject(nullptr);
                delete test;
                componentList.erase(component);
                return true;
            }
        }
        return false;
    }
    template<typename T>
    T* getComponent()
    {
        for(auto component : componentList)
        {
            T* ret = dynamic_cast<T*>(component);
            if (ret) return ret;
        }
        return nullptr;
    }

    void addChild(GameObject* go);
	void destroy();
	void hideAll();
    bool isChildOf(GameObject* go) const;
    GameObject* findChildByName(const std::string& name);
    void setName(const std::string& name);
    std::string getName() const;
	void setID(const int ID);
	int getID() const;

	void debugDraw();
    void update(float deltaTime);
    void fixedUpdate();
    void collisionEnter(GameObject* other);
    void collisionStay(GameObject* other);
    void collisionExit(GameObject* other);

	void extract();

	std::vector<std::vector<char>> serializeCreation(int parentID);

	enum DeserializeCreateResult { SUCCESS=0, ID_ALREADY_EXISTS, NO_PARENT_FOUND };
	DeserializeCreateResult deserializeAndCreate(std::vector<char> bytes);

protected:
    bool dead, newlyCreated;
    std::vector<Component*> componentList;
    std::string name;
	int ID;
	static std::multimap<std::string, GameObject*> nameMap;
	static std::multimap<int, GameObject*> idMap;
	void removeName();
	void removeID();
};

#endif

