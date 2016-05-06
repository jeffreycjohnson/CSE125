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
	RayHitInfo raycast(const Ray& ray, Octree::BuildMode whichTree, float t_min = Octree::RAY_MIN, float t_max = Octree::RAY_MAX);

	// Returns any collisions that occur against the axis-aligned bounding box defined by the two min & max points
	CollisionInfo collisionBox(glm::vec3 min, glm::vec3 max, Octree::BuildMode whichTree);

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

