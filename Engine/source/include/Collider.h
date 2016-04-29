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
	std::set<GameObject*> previousColliders; // Ptrs to gameObjects we collided with last frame

	// Given the collisions we have found at frame N, we will want to add these
	// to be in the set of "previousColliders" for N + 1
	void addPreviousColliders(const CollisionInfo& colInfo) {
		for (auto collidee : colInfo.collidees ) {
			previousColliders.insert(collidee);
		}
	};

	// Removes GameObject from the previous set
	void removePreviousColliders(const CollisionInfo& colInfo) {
		for (auto collidee : colInfo.collidees) {
			previousColliders.erase(collidee);
		}
	}

	std::vector<GameObject*> getCollisionExitEvents(const CollisionInfo& collisionsThisFrame) {
		std::vector<GameObject*> triggerExitOnThese;
		for (auto gameObject : collisionsThisFrame.collidees) {
			if (previousColliders.find(gameObject) == previousColliders.end()) {
				// If we have find a game object in our previous colliders list that is not
				// colliding with us this frame, we need to fire an exit event on that game object.
				triggerExitOnThese.push_back(gameObject);
			}
		}
		return triggerExitOnThese;
	}

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

	virtual void fixedUpdate() = 0;
	virtual void debugDraw() = 0;

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