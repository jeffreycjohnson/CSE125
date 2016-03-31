 #ifndef INCLUDE_COMPONENT_H
#define INCLUDE_COMPONENT_H

#include "ForwardDecs.h"

class Component
{
    friend class GameObject;
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
		virtual void debugDraw();
        virtual void create(); // (called on first update after fully constructed)
        virtual void destroy(); // (called before any components, children, parents, or siblings are actually destroyed)
        virtual void collisionEnter(GameObject* other);
        // TODO : IMPLEMENT
        /*virtual void collisionExit(GameObject* other);
        virtual void collisionStay(GameObject* other);
        virtual void activate();
        virtual void deactivate();*/
};

#endif