#include "OctreeManager.h"
#include "Collision.h"
#include "Collider.h"

OctreeManager::OctreeManager()
{
	staticObjects = nullptr;
	dynamicObjects = nullptr;
}


OctreeManager::~OctreeManager()
{
	if (staticObjects != nullptr)
		delete staticObjects;

	if (dynamicObjects != nullptr)
		delete dynamicObjects;
}

void OctreeManager::buildStaticOctree(const glm::vec3& min, const glm::vec3& max) {
	if (staticObjects != nullptr) {
		staticObjects = new Octree(min, max);
	}
	else {
		delete staticObjects;
		staticObjects = new Octree(min, max);
	}
	staticObjects->build(Octree::STATIC_ONLY); // GameObject.SceneRoot is the root by default
};

void OctreeManager::buildDynamicOctree(const glm::vec3& min, const glm::vec3& max) {
	if (dynamicObjects != nullptr) {
		dynamicObjects = new Octree(min, max);
	}
	else {
		delete dynamicObjects;
		dynamicObjects = new Octree(min, max);
	}
	dynamicObjects->build(Octree::DYNAMIC_ONLY); // GameObject.SceneRoot is the root by default
};

void OctreeManager::probeForStaticCollisions() {
	
	// Iterates through all the colliders in the dynamic octree, and checks
	// to see if they collide with any static objects in the static octree.
	// Collect all of the collision data into the staticCollisions vector

	if (staticObjects != nullptr && dynamicObjects != nullptr) {
		for (auto nodeIter = dynamicObjects->begin(); nodeIter != dynamicObjects->end(); ++nodeIter) {
			OctreeNode* node = (*nodeIter).second;
			if (node != nullptr) {
				
				for (auto colliderIter = node->begin(); colliderIter != node->end(); ++colliderIter) {
		
					CollisionInfo colInfo = staticObjects->collidesWith(*colliderIter);
					staticCollisions.push_back(colInfo);
					if (colInfo.numCollisions > 0) {
						staticCollisions.push_back(colInfo); // [static] Enter or Stay
					}
					else {
						colInfo.collider->colliding = false;
						if (colInfo.collider->previouslyColliding == true) {
							// staticCollisionExit() event should be fired here
							staticCollisions.push_back(colInfo);
						}
					}
				
				}
			}
		}
	}
};

void OctreeManager::probeForDynamicCollisions() {

	// Iterates through all the colliders in the dynamic octree, and checks
	// to see if they collide with other objects in the dynamics octree.
	// Collect all of the collision data into the staticCollisions vector

	if (staticObjects != nullptr && dynamicObjects != nullptr) {
		for (auto nodeIter = dynamicObjects->begin(); nodeIter != dynamicObjects->end(); ++nodeIter) {
			OctreeNode* node = (*nodeIter).second;
			if (node != nullptr) {

				for (auto colliderIter = node->begin(); colliderIter != node->end(); ++colliderIter) {

					CollisionInfo colInfo = dynamicObjects->collidesWith(*colliderIter);
					if (colInfo.numCollisions > 0) {
						dynamicCollisions.push_back(colInfo); // Enter or Stay
					}
					else {
						colInfo.collider->colliding = false;
						if (colInfo.collider->previouslyColliding == true) {
							// CollisionExit() event should be fired here
							dynamicCollisions.push_back(colInfo);
						}
					}

				}
			}
		}
	}

};

void OctreeManager::updateDynamicObjectsInOctree() {

};

void OctreeManager::beforeFixedUpdate() {

	// When this function is called, we assume that all colliders are in the state
	// (e.g. position) for frame N - 1.

	// Soon, they will process their updates, but we don't want that to happen until
	// they have processed their collision/triggerXXXX() methods (because objects that,
	// moved into a wall, for example, should have been pushed back in the previous frame)

	// 1. Since octrees are in state N - 1, look for all the collisions that occurred
	//    during frame N - 1
	probeForStaticCollisions();
	probeForDynamicCollisions();

	// 2. Call appropriate collision methods (based on the booleans in Collider.h) so that
	//    they are processed before the next fixedUpdate()

	for (auto collisionData : staticCollisions) {
		// Since the colliders we have access to here are DYNAMIC objects, we want
		// to make sure that we update their position in the dynamic octree AFTER
		// we call the staticCollisionXXXX() method.
		GameObject* caller = collisionData.collider->gameObject;

		// state @ N - 1
		bool colliding = collisionData.collider->colliding = true;
		
		// state @ N - 2
		bool previouslyColliding = collisionData.collider->previouslyColliding; 

		// /!\ Warning: the following code is black magic. Exercise caution.

		for (auto other : collisionData.collidees) {
			if (colliding && previouslyColliding) {
				// Collision Stay
				caller->collisionCallback(other, &Component::staticCollisionStay);
			}
			else if (colliding && !previouslyColliding) {
				// Collision Enter
				caller->collisionCallback(other, &Component::staticCollisionEnter);
			}
			else if (!colliding && previouslyColliding) {
				// Collision Exit
				caller->collisionCallback(other, &Component::staticCollisionExit);
			}
		}

		// *** end of black magic ***
	}

	// 3. We explicitly check 

};

void OctreeManager::fixedUpdate() {
	// Not sure if we'll need to do stuff here
};

void OctreeManager::afterFixedUpdate() {
	// Before the fixed update, we did all the collision detection for N - 1,
	// but the previouslyCollding boolean reflected the colliding state of N - 2,
	// so we need to update it to whatever happened *this* frame.
	for (auto collision : staticCollisions) {
		collision.collider->previouslyColliding = collision.collider->colliding;
	}
	for (auto collision : dynamicCollisions) {
		collision.collider->previouslyColliding = collision.collider->colliding;
	}
};

void OctreeManager::debugDraw() {
	if (staticObjects != nullptr) {
		staticObjects->debugDraw();
	}
	if (dynamicObjects != nullptr) {
		dynamicObjects->debugDraw();
	}
}