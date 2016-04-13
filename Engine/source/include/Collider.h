#pragma once

#include "ForwardDecs.h"
#include "Component.h"
#include "Collision.h"

class Collider : public Component {
private:
	friend OctreeNode;
	NodeId nodeId = Octree::UNKNOWN_NODE;
	bool colliding;

public:
	bool passive;

	virtual ~Collider();
	
	virtual void destroy();
	virtual void update(float);
	virtual void debugDraw();
	virtual void onCollisionEnter(GameObject* other);

	virtual bool intersects(const BoxCollider& other) = 0; 
	//virtual bool intersects(const CapsuleCollider& other) = 0;
	//virtual bool intersects(const SphereCollider& other) = 0;
};