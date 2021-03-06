#include "OctreeManager.h"
#include "Collision.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "Camera.h"
#include "Renderer.h"
#include "RenderPass.h"
#include <iostream>

OctreeManager::OctreeManager()
{
	staticObjects = nullptr;
	dynamicObjects = nullptr;
	dynamicCollisionsThisFrame = staticCollisionsThisFrame = 0;
}


OctreeManager::~OctreeManager()
{
	if (staticObjects != nullptr)
		delete staticObjects;

	if (dynamicObjects != nullptr)
		delete dynamicObjects;
}

RayHitInfo OctreeManager::raycast(const Ray & ray, Octree::BuildMode whichTree, float t_min, float t_max, std::initializer_list<Collider*> ignore, bool ignoreTriggers)
{
	if (glm::isnan(ray.direction).b) {
		return RayHitInfo(); // No NaN(s) allowed.
	}

	if (whichTree == Octree::DYNAMIC_ONLY) {
		return dynamicObjects->raycast(ray, t_min, t_max, ignore, ignoreTriggers);
	}
	else if (whichTree == Octree::STATIC_ONLY) {
		return staticObjects->raycast(ray, t_min, t_max, ignore, ignoreTriggers);
	}
	else {
		auto dynaHit = dynamicObjects->raycast(ray, t_min, t_max, ignore, ignoreTriggers);
		auto statHit = staticObjects->raycast(ray, t_min, t_max, ignore, ignoreTriggers);
		if (dynaHit.intersects && statHit.intersects) {
			if (dynaHit.hitTime < statHit.hitTime) {
				return dynaHit;
			}
			// TODO: Handle the "tie" case more gracefully. Add more detailed info to RayHitInfo instead of lazily giving up and picking one?
			else {
				return statHit;
			}
		}
		else if (dynaHit.intersects && !statHit.intersects) {
			return dynaHit;
		}
		else if (!dynaHit.intersects && statHit.intersects) {
			return statHit;
		}
		else {
			return statHit; // Default ray hit obj should be false
		}
	}
}

CollisionInfo OctreeManager::collisionBox(glm::vec3 min, glm::vec3 max, Octree::BuildMode whichTree)
{
	BoxCollider temp(glm::vec3(0), glm::vec3(0));
	temp.setMinAndMax(min, max);
	if (whichTree == Octree::BuildMode::STATIC_ONLY) {
		return staticObjects->collidesWith(&temp);
	}
	else if (whichTree == Octree::BuildMode::DYNAMIC_ONLY) {
		return dynamicObjects->collidesWith(&temp);
	}
	else {
		auto s_cols = staticObjects->collidesWith(&temp);
		auto d_cols = dynamicObjects->collidesWith(&temp);
		s_cols.merge(d_cols);
		return s_cols;
	}
}

CollisionInfo OctreeManager::collisionBox(BoxCollider * box, Octree::BuildMode whichTree)
{
	if (whichTree == Octree::BuildMode::STATIC_ONLY) {
		return staticObjects->collidesWith(box);
	}
	else if (whichTree == Octree::BuildMode::DYNAMIC_ONLY) {
		return dynamicObjects->collidesWith(box);
	}
	else {
		auto s_cols = staticObjects->collidesWith(box);
		auto d_cols = dynamicObjects->collidesWith(box);
		s_cols.merge(d_cols);
		return s_cols;
	}
}

void OctreeManager::insertGameObject(GameObject *gameObject)
{

	if (gameObject != nullptr && dynamicObjects != nullptr && staticObjects != nullptr) {
		Collider *a, *b, *c;
		a = (Collider*)gameObject->getComponent<BoxCollider>();
		b = (Collider*)gameObject->getComponent<SphereCollider>();
		c = (Collider*)gameObject->getComponent<CapsuleCollider>();

		if (a != nullptr) {
			dynamicObjects->insert(a);
			staticObjects->insert(a);
		}
		if (b != nullptr) {
			dynamicObjects->insert(b);
			staticObjects->insert(b);
		}
		if (c != nullptr) {
			dynamicObjects->insert(c);
			staticObjects->insert(c);
		}

		// Recurse!
		for (auto child : gameObject->transform.children) {
			insertGameObject(child->gameObject);
		}

	}
}

void OctreeManager::removeGameObject(GameObject *gameObject)
{
	if (gameObject != nullptr && dynamicObjects != nullptr && staticObjects != nullptr) {
		Collider *a, *b, *c;
		a = (Collider*)gameObject->getComponent<BoxCollider>();
		b = (Collider*)gameObject->getComponent<SphereCollider>();
		c = (Collider*)gameObject->getComponent<CapsuleCollider>();

		if (a != nullptr) {
			dynamicObjects->remove(a);
			staticObjects->remove(a);
		}
		if (b != nullptr) {
			dynamicObjects->remove(b);
			staticObjects->remove(b);
		}
		if (c != nullptr) {
			dynamicObjects->remove(c);
			staticObjects->remove(c);
		}

		// Recurse!
		for (auto child : gameObject->transform.children) {
			removeGameObject(child->gameObject);
		}
	}
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
			OctreeNode* node = nodeIter->second;
			if (node != nullptr) {
				
				for (auto colliderIter = node->begin(); colliderIter != node->end(); ++colliderIter) {
		
					CollisionInfo colInfo = staticObjects->collidesWith(*colliderIter);
					if (colInfo.numCollisions > 0) {
						colInfo.collider->collidingStatic = true;
						colInfo.collider->addPreviousColliders(colInfo);
						staticCollisions.push_back(colInfo); // [static] Enter or Stay
						staticCollisionsThisFrame += colInfo.numCollisions;
					}
					else {
						colInfo.collider->collidingStatic = false;
						if (colInfo.collider->previouslyCollidingStatic == true) {
							// staticCollisionExit() event should be fired here
							//colInfo.collider->removePreviousColliders(colInfo);
							staticCollisions.push_back(colInfo); // TODO: ignore collision exit for now
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
						colInfo.collider->colliding = true;
						colInfo.collider->addPreviousColliders(colInfo);
						dynamicCollisions.push_back(colInfo); // Enter or Stay
						dynamicCollisionsThisFrame += colInfo.numCollisions;
					}
					else {
						colInfo.collider->colliding = false;
						if (colInfo.collider->previouslyColliding == true) {
							// CollisionExit() event should be fired here
							dynamicCollisions.push_back(colInfo); // TODO: Ignore collision exit for now
						}
					}

				}
			}
		}
	}

};

void OctreeManager::updateDynamicObjectsInOctree() {
	if (NetworkManager::getState() == NetworkState::CLIENT_MODE) return;
	dynamicObjects->rebuild();
}

void OctreeManager::beforeFixedUpdate() {
	if (NetworkManager::getState() == NetworkState::CLIENT_MODE) return;
	// When this function is called, we assume that all colliders are in the state
	// (e.g. position) for frame N - 1.

	// Soon, they will process their updates, but we don't want that to happen until
	// they have processed their collision/triggerXXXX() methods (because objects that,
	// moved into a wall, for example, should have been pushed back in the previous frame)

	// 1. Since octrees are in state N - 1, look for all the collisions that occurred
	//    during frame N - 1

	dynamicCollisionsThisFrame = 0;
	staticCollisionsThisFrame = 0;
	probeForStaticCollisions();
	probeForDynamicCollisions();

	// 2. Call appropriate collision methods (based on the booleans in Collider.h) so that
	//    they are processed before the next fixedUpdate()

	for (auto collisionData : staticCollisions) {
		// Since the colliders we have access to here are DYNAMIC objects, we want
		// to make sure that we update their position in the dynamic octree AFTER
		// we call the staticCollisionXXXX() method.
		GameObject* caller = collisionData.collider->gameObject;

		// Since we load each node in blender as a single gameobject, we actually want
		// to trigger the collision up two levels
		// --> obj_Player
		// -----> Colliders
		// ---------> BoxCollider

		caller = caller->transform.getParent()->getParent()->gameObject;

		// state @ N - 1
		bool colliding = collisionData.collider->collidingStatic;

		// state @ N - 2
		bool previouslyColliding = collisionData.collider->previouslyCollidingStatic;

		// /!\ Warning: the following code is black magic. Exercise caution.

		for (auto other : collisionData.collidees) {
			other = other->transform.getParent()->getParent()->gameObject;
			if (colliding && previouslyColliding) {
				// Static Collision Stay
				caller->collisionCallback(other, &Component::staticCollisionStay);
			}
			else if (colliding && !previouslyColliding) {
				// Static Collision Enter
				caller->collisionCallback(other, &Component::staticCollisionEnter);
			}
		}

		if (!colliding && previouslyColliding) {
			// Static Collision Exit
			auto previouslyColliding = collisionData.collider->getCollisionExitEvents(collisionData);
			for (auto other : previouslyColliding) {
				caller->collisionCallback(other, &Component::staticCollisionExit);
			}
		}

		// *** end of black magic ***
	}

	// For now we're doing static & dynamic at the same time. It seems like this
	// *could* be bad, but otherwise we'd need to rebuild the octree twice per frame
	// as opposed to once, and honestly, doing them at the same time might not matter
	// as much as one would think

	for (auto collisionData : dynamicCollisions) {
		GameObject* caller = collisionData.collider->gameObject;

		// Since we load each node in blender as a single gameobject, we actually want
		// to trigger the collision up two levels
		// --> obj_Player
		// -----> Colliders
		// ---------> BoxCollider

		caller = caller->transform.getParent()->getParent()->gameObject;

		// state @ N - 1
		bool colliding = collisionData.collider->colliding;

		// state @ N - 2
		bool previouslyColliding = collisionData.collider->previouslyColliding;

		for (auto other : collisionData.collidees) {
			other = other->transform.getParent()->getParent()->gameObject;
			if (colliding && previouslyColliding) {
				// Collision Stay
				caller->collisionCallback(other, &Component::collisionStay);
			}
			else if (colliding && !previouslyColliding) {
				// Collision Enter
				caller->collisionCallback(other, &Component::collisionEnter);
			}
		}

		if (!colliding && previouslyColliding) {
			// Collision Exit
			auto previouslyColliding = collisionData.collider->getCollisionExitEvents(collisionData);
			for (auto other : previouslyColliding) {
				caller->collisionCallback(other, &Component::collisionExit);
			}
		}
	}

};

void OctreeManager::fixedUpdate() {

};

void OctreeManager::afterFixedUpdate() {
	if (NetworkManager::getState() == NetworkState::CLIENT_MODE) return;

	// Before the fixed update, we did all the collision detection for N - 1,
	// but the previouslyCollding boolean reflected the colliding state of N - 2,
	// so we need to update it to whatever happened *this* frame.
	for (auto collision : staticCollisions) {
		collision.collider->previouslyCollidingStatic = collision.collider->collidingStatic;
		collision.collider->removePreviousColliders(collision); // TODO: this was broken anyway, but it's more broken b/c it doesn't separate Dynamic & Static collisions
		collision.collider->addPreviousColliders(collision);
	}
	for (auto collision : dynamicCollisions) {
		collision.collider->previouslyColliding = collision.collider->colliding;
		collision.collider->removePreviousColliders(collision);
		collision.collider->addPreviousColliders(collision);
	}

	// Print out collision data
	// Flush the collisions buffer
	dynamicCollisions.clear();
	staticCollisions.clear();

	updateDynamicObjectsInOctree();

};

void OctreeManager::debugDraw() {
	if (staticObjects != nullptr && DebugPass::drawStaticOctree) {
		staticObjects->debugDraw();
	}
	if (dynamicObjects != nullptr && DebugPass::drawDynamicOctree) {
		dynamicObjects->debugDraw();
	}
	//Renderer::drawSphere(glm::vec3(0), 1.0f, glm::vec4(1)); // draw sphere @ origin
}
