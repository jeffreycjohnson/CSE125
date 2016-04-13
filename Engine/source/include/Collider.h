#pragma once

#include "ForwardDecs.h"
#include "Component.h"
#include "Collision.h"

class Collider : public Component {
private:
	friend OctreeNode;
	NodeId nodeId = Octree::UNKNOWN_NODE;
	Octree* octree;
	bool colliding;

public:
	bool passive;

	virtual ~Collider() {
		// TODO: Remove this collider from the octree
	};
	
	virtual void destroy() = 0;
	virtual void update(float) = 0;
	virtual void debugDraw() = 0;
	virtual void onCollisionEnter(GameObject* other) = 0;

	virtual bool intersects(const BoxCollider& other) = 0; 
	//virtual bool intersects(const CapsuleCollider& other) = 0;
	//virtual bool intersects(const SphereCollider& other) = 0;

	virtual BoxCollider getAABB() = 0;
};