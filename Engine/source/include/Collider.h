#pragma once

#include "ForwardDecs.h"
#include "Component.h"
#include "Collision.h"
#include "OctreeManager.h"

enum ColliderType {
	UNKNOWN,   // Unknown
	AABB,
	OBB,
	SPHERE,
	CAPSULE
};

class Collider : public Component {
protected:
	friend Octree;
	friend OctreeNode;
	friend OctreeManager;

	NodeId nodeId = Octree::UNKNOWN_NODE;
	Octree* octree;

	bool previouslyColliding; // Colliding during frame N
	bool colliding;           // Colliding during frame N + 1

public:
	bool passive; // Should be set to TRUE if the object is static; false otherwise

	virtual ~Collider() {};
	
	virtual void destroy() {
		if (octree != nullptr) {
			octree->remove(this);
			octree = nullptr;
			nodeId = Octree::UNKNOWN_NODE;
		}
	};

	virtual void setStatic(bool isStatic) {
		passive = isStatic;
	};

	virtual void update(float) = 0;
	virtual void debugDraw() = 0;
	virtual void onCollisionEnter(GameObject* other) = 0; // TODO: Probably will not need this

	virtual bool insideOrIntersects(const glm::vec3& point) const = 0;
	virtual bool intersects(const BoxCollider& other) const = 0; 
	virtual bool intersects(const CapsuleCollider& other) const = 0;
	virtual bool intersects(const SphereCollider& other) const = 0;

	// Returns an axis-aligned bounding box defined for WORLD coordinates
	virtual BoxCollider getAABB() const = 0;
	virtual ColliderType getColliderType() {
		return ColliderType::UNKNOWN;
	}
};