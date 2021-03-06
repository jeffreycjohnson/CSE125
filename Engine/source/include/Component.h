 #ifndef INCLUDE_COMPONENT_H
#define INCLUDE_COMPONENT_H

#include "ForwardDecs.h"
#include <vector>

class Component
{
	friend class GameObject;

private:
    bool newlyCreated = true;

public:
	GameObject* gameObject = nullptr;
	bool visible = true;
	bool active = true;

	virtual ~Component() = default;
	virtual void setGameObject(GameObject* go);

	virtual void update(float deltaTime);
	virtual void fixedUpdate();

	virtual void draw();
	virtual bool drawUI();
	virtual void debugDraw();
	virtual void create(); // (called on first update after fully constructed)
	virtual void destroy(); // (called before any components, children, parents, or siblings are actually destroyed)

	/*
		collisionXXXX()         methods called on dynamic objects when they collide with other dynamic objects
		staticCollisionXXXX()   methods called on dynamic objects when they collide with static objects
	*/

	virtual void collisionEnter(GameObject* other);
	virtual void collisionExit(GameObject* other);
	virtual void collisionStay(GameObject* other);
	
	virtual void staticCollisionEnter(GameObject* other);
	virtual void staticCollisionStay(GameObject* other);
	virtual void staticCollisionExit(GameObject* other);
	/*
	virtual void activate();
	virtual void deactivate();*/

	// serialization
	virtual std::vector<char> serialize();
	virtual void deserializeAndApply(std::vector<char> bytes);
};

#endif
