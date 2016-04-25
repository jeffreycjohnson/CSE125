#pragma once
#include "Component.h"
#include "Collision.h"

class OctreeManager :
	public Component
{
private:
	Octree* staticObjects;
	Octree* dynamicObjects;
	bool useNaive; // TODO: make this configurable
	int dynamicCollisionsThisFrame, staticCollisionsThisFrame;

	std::vector<CollisionInfo> staticCollisions;
	std::vector<CollisionInfo> dynamicCollisions;

	std::vector<Collider*> objects; // New naive implementation

	void probeForStaticCollisions();
	void probeForDynamicCollisions();
	void updateDynamicObjectsInOctree();
	
	void naiveCollisionDetection();

public:
	OctreeManager();
	~OctreeManager();

	// Adds or removes gameobjects (and all their colliders) to the
	// apropriate octree. Will recursively add children as well.
	void insertGameObject(GameObject*);
	void removeGameObject(GameObject*);

	void buildStaticOctree(const glm::vec3&, const glm::vec3&);
	void buildDynamicOctree(const glm::vec3&, const glm::vec3&);

	void beforeFixedUpdate() override;
	void fixedUpdate() override;
	void afterFixedUpdate() override;

	void debugDraw() override;

};

