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

void OctreeManager::probeForStaticCollisions() {
	
	// Iterates through all the colliders in the static octree, and checks
	// to see if they collide with dynamics objects in the dynamics octree.
	// Collect all of the collision data into the staticCollisions vector

	if (staticObjects != nullptr && dynamicObjects != nullptr) {
		for (auto nodeIter = staticObjects->begin(); nodeIter != staticObjects->end(); ++nodeIter) {
			OctreeNode* node = (*nodeIter).second;
			if (node != nullptr) {
				for (auto colliderIter = node->begin(); colliderIter != node->end(); ++colliderIter) {
					// TODO: Refactor octree->collidesWith to take collider *
					//CollisionInfo colInfo = dynamicObjects->collidesWith(*colliderIter);
					//staticCollisions.push_back(colInfo);
				}
			}
		}
	}
};

void OctreeManager::probeForDynamicCollisions() {

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
	// 2. Call appropriate collision methods (based on the booleans in Collider.h) so that
	//    they are processed before the next fixedUpdate()
};

void OctreeManager::fixedUpdate() {
	// Not sure if we'll need to do stuff here
};

void OctreeManager::afterFixedUpdate() {
	// Now that we've processed the fixedUpdate() for all GameObjects, we want to
	// check if any new collisions have occurred.
};