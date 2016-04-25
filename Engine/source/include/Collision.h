/*
 The entire collision detection system goes here.

 - Implements an octree collision/culling detection system
 - Implements a high-level interface for accessing the result of intersection
   tests (CollisionInfo)
 - (WILL) implement Raycasting

 \/ Inspiration for handling octree edge cases
 https://geidav.wordpress.com/2014/11/18/advanced-octrees-3-non-static-octrees/
 
 */
#pragma once
#include "ForwardDecs.h"
#include "GameObject.h"
#include <set>
#include <vector>
#include <unordered_map>

// Because 'unsigned long' is super vague. Don't use this outside
// of the collision detection system.
typedef unsigned long NodeId;

class Octree {
public:
	friend OctreeNode;
	Octree(glm::vec3 min, glm::vec3 max);
	~Octree();

	/* Maximum number of colliders allowed inside a single OctreeNode */
	static const int LEAF_THRESHOLD = 10;
	static const int CHILDREN = 8;
	static const int MAX_DEPTH = 12;
	static const float RAY_MIN;
	static const float RAY_MAX;
	static const float RAY_STEP;
	static const NodeId UNKNOWN_NODE = 0; // First real node has ID = 1

	//static Octree* STATIC_TREE;
	//static Octree* DYNAMIC_TREE;

	enum BuildMode {
		STATIC_ONLY,  // Only includes colliders with passive = TRUE
		DYNAMIC_ONLY, // Only includes colliders with passive = FALSE
		BOTH          // Errythand. (DO NOT DO THIS, YOU WILL HAVE REGRETS!)
	};

	// --- Member Functions ---

	// Inserts a collider into the octree, and updating data in the collider
	void insert(Collider*);
	void remove(Collider*);

	void debugDraw();

	// Creates an octree, starting at the given root
	void build(BuildMode mode = BOTH, const GameObject& root = GameObject::SceneRoot);

	// Removes all colliders from the octree & reinserts them at the root (no nodes are destroyed)
	// Preserves the min/max and BuildMode restrictions from build
	void rebuild();

	CollisionInfo raycast(Ray, float min_t = RAY_MIN, float max_t = RAY_MAX, float step = Octree::RAY_STEP);
	CollisionInfo collidesWith(Collider*);

	/* I'm afraid of storing pointers inside of BoxColliders, in case things get deleted on-the-fly. */
	OctreeNode* getNodeById(NodeId id);

	/* Allow iterating through all of the nodes */
	std::unordered_map<NodeId, OctreeNode*>::iterator begin();
	std::unordered_map<NodeId, OctreeNode*>::iterator end();

private:
	OctreeNode* root;
	NodeId nodeCounter = UNKNOWN_NODE;
	std::unordered_map<NodeId, OctreeNode*> nodeMap;
	int objects;

	// Every time we call build() we reset this. It essentially short-circuits collider
	// insertion based on the Collider's passive bool.
	BuildMode restriction;

	// OctreeNode(s) notify the Octree whenever they are created/destroyed
	void removeNode(NodeId node);
	void addNode(NodeId node, OctreeNode* self);

};

/*
 A node in the Octree. OctreeNodes each have a given NodeId, so they can quickly be
 looked up from the Octree. This also avoids storing pointers in BoxColliders, and then
 having to notify BoxColliders when those pointers are freed.

 PROPERTIES

 - Every octree node that is not a leaf, has 8 octree children.
 - Every object
 - Leaf nodes cannot have more than LEAF_THRESHOLD # of colliders, UNLESS
   the maximum recursion depth has been exceeded.
 
 */
class OctreeNode {
public:
	friend Octree;

	OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent = nullptr, int depth = 0);
	~OctreeNode();

	bool isLeaf() const;
	std::string toString() const;

	// Returns an iterator into the colliders list
	std::vector<Collider*>::iterator begin();
	std::vector<Collider*>::iterator end();

private:
	std::vector<OctreeNode*> children;
	glm::vec3 min, max;
	Octree* tree;
	OctreeNode* parent;
	BoxCollider* myAABB;
	NodeId nodeId;
	int depth;

	/* Only leaf nodes should contain colliders */
	std::vector<Collider*> colliders;

	/* Member Functions */

	CollisionInfo raycast(const Ray&);
	CollisionInfo collidesWith(const BoxCollider&, CollisionInfo&);
	CollisionInfo collidesWith(const CapsuleCollider&, CollisionInfo&);
	CollisionInfo collidesWith(const SphereCollider&, CollisionInfo&);
	
	// Add or remove nodes to the data structure
	bool insert(Collider* colliderBeingInserted, const BoxCollider&); // Returns true if the node was successfully inserted
	void remove(Collider* colliderBeingRemoved);

	// Used internally for inserting/moving colliders around the octree
	bool intersects(const BoxCollider&);
	void subdivide();

	//void collapseIntoParent(); // Uhheeeh ummm,... figure this out
	void debugDraw();

};

/*
  We will likely run into scenarios where one collider will collide
  with multiple objects. This class will be used to store potentially
  relevant information when collisions occur.
*/
class CollisionInfo {
	friend OctreeNode;

public:
	CollisionInfo();
	~CollisionInfo();
	bool collisionOccurred;
	int numCollisions;

//private:
	Collider* collider; // The collider upon which collisionXXXX() will be called
	std::set<GameObject*> collidees;
	void add(Collider*);
	void merge(const CollisionInfo&);
};

/*
 A simple ray class, for all your raycasting needs.
 */
class Ray {
public:
	glm::vec3 origin, direction;
	float t;

	Ray(glm::vec3 o, glm::vec3 d);

	// Returns a discrete point along the ray at the timestep t
	glm::vec3 getCurrentPosition() const;
	glm::vec3 getPos(float tt) const;
};