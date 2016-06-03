#pragma once
#include "Component.h"
#include "Collision.h"

class OctreeManager :
	public Component
{
private:
	Octree* staticObjects;
	Octree* dynamicObjects;
	int dynamicCollisionsThisFrame, staticCollisionsThisFrame;

	std::vector<CollisionInfo> staticCollisions;
	std::vector<CollisionInfo> dynamicCollisions;

	void probeForStaticCollisions();
	void probeForDynamicCollisions();
	void updateDynamicObjectsInOctree();
	
public:
	OctreeManager();
	~OctreeManager();

	// Gameplay Collision detection API
	// ray            - ray we are casting against objects
	// whichTree      - determines whether we are casting against DYNAMIC, STATIC or BOTH kinds of objects
	// t_min          - minimum t we will accept
	// t_max          - maximum t we will accept
	// ignoreCollider - raycast will ignore this object, even if it is the earliest collision
	// ignoreTriggers - if true, raycast will ignore all colliders with "trigger" in their name
	RayHitInfo raycast(const Ray& ray, Octree::BuildMode whichTree, float t_min = Octree::RAY_MIN, float t_max = Octree::RAY_MAX, std::initializer_list<Collider*> ignore = {}, bool ignoreTriggers = true);

	// Returns any collisions that occur against the axis-aligned bounding box defined by the two min & max points
	CollisionInfo collisionBox(glm::vec3 min, glm::vec3 max, Octree::BuildMode whichTree);

	// Same thing as above, but allows checking an arbitrary boxes (including OBBs)
	CollisionInfo collisionBox(BoxCollider* box, Octree::BuildMode whichTree);

	// Adds or removes gameobjects (and all their colliders) to the
	// apropriate octree. Will recursively add children as well.
	void insertGameObject(GameObject*);
	void removeGameObject(GameObject*);

	void buildStaticOctree(const glm::vec3&, const glm::vec3&);
	void buildDynamicOctree(const glm::vec3&, const glm::vec3&);

	// These 2 functions are hooked into UpdateScene() by two lambda
	// expressions, which can be found in LoadOctreeOptions() in 
	//     Engine/source/Main.cpp
	void beforeFixedUpdate();
	void afterFixedUpdate();

	void fixedUpdate() override;

	void debugDraw() override;

};

