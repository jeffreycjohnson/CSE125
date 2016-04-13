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
#include <vector>
#include <unordered_map>

// Because 'unsigned long' is super vague. Don't use this outside
// of the collision detection system.
typedef unsigned long NodeId;

class Octree {
public:
	friend OctreeNode;
	Octree();
	~Octree();

	/* Maximum number of colliders allowed inside a single OctreeNode */
	static const int LEAF_THRESHOLD = 10;
	static const int CHILDREN = 8;
	static const int MAX_DEPTH = 16;
	static const NodeId UNKNOWN_NODE = 0; // First real node has ID = 1

	// Member Functions

	// Inserts a collider into the octree, and updating data in the collider
	void insert(BoxCollider&);
	void remove(BoxCollider&);

	void debugDraw();

	CollisionInfo raycast(const Ray&);
	CollisionInfo collidesWith(const BoxCollider&);

	/* I'm afraid of storing pointers inside of BoxColliders, in case things get deleted on-the-fly. */
	OctreeNode* getNodeById(NodeId id);

private:
	OctreeNode* root;
	NodeId nodeCounter = UNKNOWN_NODE;
	std::unordered_map<NodeId, OctreeNode*> nodeMap;

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

	OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth);
	~OctreeNode();

	CollisionInfo raycast(const Ray&);
	CollisionInfo collidesWith(const BoxCollider&);
	bool isLeaf() const;

private:
	std::vector<OctreeNode*> children;
	glm::vec3 min, max;
	Octree* tree;
	OctreeNode* parent;
	NodeId nodeId;
	int depth;

	/* Only leaf nodes should contain colliders */
	std::vector<BoxCollider*> colliders;

	/* Member Functions */

	// Add or remove nodes to the data structure
	void insert(BoxCollider&);
	void remove(BoxCollider&);

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

private:
	std::vector<GameObject*> collidees;
};

/*
 A simple ray class, for all your raycasting needs.
 */
class Ray {
public:
	glm::vec3 origin, direction;
	float t;

	Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {}
};