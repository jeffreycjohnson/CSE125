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
	static int objectIDCounter;
    static GameObject SceneRoot;
	static GameObject* FindByName(const std::string& name);
	static GameObject* FindByID(const int& id);
	static std::vector<GameObject*> FindAllByName(const std::string& name);
	static std::vector<GameObject*> FindAllByPrefix(const std::string& name);
	static void UpdateScene(int caller);

	// registering callbacks for updates
	static void AddPreFixedUpdateCallback(void(*callback)(void));
	static void AddPostFixedUpdateCallback(void(*callback)(void));
	static void AddPreUpdateCallback(void(*callback)(void));
	static void AddPostUpdateCallback(void(*callback)(void));

	static void DestroyObjectByID(int objectID);

	static void Dispatch(const std::vector<char> &bytes, int messageType, int messageID);

	GameObject();
	GameObject(int id, std::string name);

	~GameObject();

    template<typename T>
    void addComponent(T* c) {
        removeComponent<T>();
        c->setGameObject(this);
        componentList.push_back(c);
    }
    template<typename T>
    bool removeComponent(bool deleteComponent = true)
    {
        for (auto component = componentList.begin(); component != componentList.end(); ++component)
        {
            T* test = dynamic_cast<T*>(*component);
            if (test)
            {
                test->setGameObject(nullptr);
				if (deleteComponent)
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
	void removeChild(GameObject* go);
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
	void beforeFixedUpdate(); // <-- Do not use this for gameplay logic!!!
    void fixedUpdate();
	void afterFixedUpdate();  // <-- Do not use this for gameplay logic!!!

	// <-- we no longer need this because I am a fucking wizard
    void collisionEnter(GameObject* other); // TODO: Remove, ALL HAIL THE DARK LORD FRIEDMAN

	// Forgive me for functional C++, but I'm too lazy to write duplicate extra functions -- Dexter
	void collisionCallback(GameObject* other, void(Component::*callback)(GameObject*));

    //void collisionStay(GameObject* other);
    //void collisionExit(GameObject* other);

	void extract();

	std::vector<char> serialize();
	static bool deserializeAndCreate(std::vector<char> bytes);
	void setVisible(bool visible);
	void setActive(bool active);
	bool getVisible();
	bool getActive();

protected:
    bool dead, newlyCreated;
	bool visible, active;

    std::vector<Component*> componentList;
    std::string name;
	int ID = -1;
	static std::multimap<std::string, GameObject*> nameMap;
    static std::map<int, GameObject*> idMap;
	void removeName();
	void removeID();

	void postToNetwork();
};

#endif

