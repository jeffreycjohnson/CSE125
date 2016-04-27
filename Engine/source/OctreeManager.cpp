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
}


OctreeManager::~OctreeManager()
{
	if (staticObjects != nullptr)
		delete staticObjects;

	if (dynamicObjects != nullptr)
		delete dynamicObjects;
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
						colInfo.collider->colliding = true;
						staticCollisions.push_back(colInfo); // [static] Enter or Stay
						staticCollisionsThisFrame += colInfo.numCollisions;
					}
					else {
						colInfo.collider->colliding = false;
						if (colInfo.collider->previouslyColliding == true) {
							// staticCollisionExit() event should be fired here
							//staticCollisions.push_back(colInfo); // TODO: ignore collision exit for now
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
						dynamicCollisions.push_back(colInfo); // Enter or Stay
						dynamicCollisionsThisFrame += colInfo.numCollisions;
					}
					else {
						colInfo.collider->colliding = false;
						if (colInfo.collider->previouslyColliding == true) {
							// CollisionExit() event should be fired here
							//dynamicCollisions.push_back(colInfo); // TODO: Ignore collision exit for now
						}
					}

				}
			}
		}
	}

};

void OctreeManager::updateDynamicObjectsInOctree() {

}

void OctreeManager::beforeFixedUpdate() {

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

		// state @ N - 1
		bool colliding = collisionData.collider->colliding;// = true; // Don't do this!

		// state @ N - 2
		bool previouslyColliding = collisionData.collider->previouslyColliding;

		// /!\ Warning: the following code is black magic. Exercise caution.

		for (auto other : collisionData.collidees) {
			if (colliding && previouslyColliding) {
				// Static Collision Stay
				caller->collisionCallback(other, &Component::staticCollisionStay);
			}
			else if (colliding && !previouslyColliding) {
				// Static Collision Enter
				caller->collisionCallback(other, &Component::staticCollisionEnter);
			}
		}

		/*if (!colliding && previouslyColliding) {
			// Static Collision Exit
			caller->collisionCallback(other, &Component::staticCollisionExit);
		}*/ // TODO: Collision Exit is special

		// *** end of black magic ***
	}

	// For now we're doing static & dynamic at the same time. It seems like this
	// *could* be bad, but otherwise we'd need to rebuild the octree twice per frame
	// as opposed to once, and honestly, doing them at the same time might not matter
	// as much as one would think

	for (auto collisionData : dynamicCollisions) {
		GameObject* caller = collisionData.collider->gameObject;

		// state @ N - 1
		bool colliding = collisionData.collider->colliding;// = true; // Don't do this!

		// state @ N - 2
		bool previouslyColliding = collisionData.collider->previouslyColliding;

		for (auto other : collisionData.collidees) {
			if (colliding && previouslyColliding) {
				// Collision Stay
				caller->collisionCallback(other, &Component::collisionStay);
			}
			else if (colliding && !previouslyColliding) {
				// Collision Enter
				caller->collisionCallback(other, &Component::collisionEnter);
			}
		}

		/*if (!colliding && previouslyColliding) {
			// Collision Exit
			caller->collisionCallback(other, &Component::collisionExit);
		}*/ // TODO: Collision Exit is special, deal with it later
	}

};

#include "Timer.h"

void OctreeManager::fixedUpdate() {

	/*Ray ray = Renderer::mainCamera->getEyeRay();
	CollisionInfo colInfo = dynamicObjects->raycast(ray, Octree::RAY_MIN, 5, 0.1f);
	if (colInfo.collisionOccurred) {
		int x = 0;
		++x; // nop
		if (Timer::nextFixedStep()) {
			std::cerr << "lel" << std::endl;
		}
	}*/

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

	// Print out collision data
	/*std::cerr << "Dynamic Collisions this frame: " << dynamicCollisions.size() << std::endl;
	std::cerr << "Static Collisions this frame:  " << staticCollisions.size() << std::endl;*/

	// Flush the collisions buffer
	dynamicCollisions.clear();
	staticCollisions.clear();

	// TODO: Profile this for a complex, actually dynamic scene
	dynamicObjects->rebuild();
};

void OctreeManager::debugDraw() {
	if (staticObjects != nullptr && DebugPass::drawStaticOctree) {
		staticObjects->debugDraw();
	}
	if (dynamicObjects != nullptr && DebugPass::drawDynamicOctree) {
		dynamicObjects->debugDraw();
	}
	Renderer::drawSphere(glm::vec3(0), 1.0f, glm::vec4(1));
}