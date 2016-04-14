#pragma once

#include "ForwardDecs.h"
#include "Component.h"
#include "Collision.h"

enum ColliderType {
	UNKNOWN,   // Unknown
	AABB,
	OBB,
	SPHERE,
	CAPSULE
};

class Collider : public Component {
protected:
	friend OctreeNode;
	NodeId nodeId = Octree::UNKNOWN_NODE;
	Octree* octree;
	bool colliding;

public:
	bool passive; // Should be set to TRUE if the object is static; false otherwise

	virtual ~Collider() {};
	
	virtual void destroy() {
		if (octree != nullptr) {
			//octree->remove(*this); // TODO: Uncomment this once Octree allows for generic Collider&
			octree = nullptr;
			nodeId = Octree::UNKNOWN_NODE;
		}
	};
	virtual void update(float) = 0;
	virtual void debugDraw() = 0;
	virtual void onCollisionEnter(GameObject* other) = 0;

	virtual bool insideOrIntersects(const glm::vec3& point) = 0;
	virtual bool intersects(const BoxCollider& other) = 0; 
	//virtual bool intersects(const CapsuleCollider& other) = 0;
	//virtual bool intersects(const SphereCollider& other) = 0;

	virtual BoxCollider getAABB() = 0;
	virtual ColliderType getColliderType() {
		return ColliderType::UNKNOWN;
	}
};