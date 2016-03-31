#ifndef INCLUDE_GAME_OBJECT_H
#define INCLUDE_GAME_OBJECT_H

#include "ForwardDecs.h"
#include "Transform.h"
#include "Component.h"
#include <vector>
#include <map>

class GameObject
{
public:
	Transform transform;
	bool visible, active;

    static GameObject SceneRoot;
    static GameObject* FindByName(const std::string& name);
    static std::vector<GameObject*> FindAllByName(const std::string& name);
    static void UpdateScene();

	GameObject();
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

	void debugDraw();
    void update(float deltaTime);
    void fixedUpdate();
    void collisionEnter(GameObject* other);
    //void collisionStay(GameObject* other);
    //void collisionExit(GameObject* other);

	void extract();

protected:
    bool dead, newlyCreated;
    std::vector<Component*> componentList;
    std::string name;
    static std::multimap<std::string, GameObject*> nameMap;
    void removeName();
};

#endif

