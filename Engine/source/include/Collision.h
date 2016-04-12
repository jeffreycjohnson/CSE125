/*
 The entire collision detection system goes here.
 */
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

	static const int MAX_DEPTH = 16;

	void insert(BoxCollider&);
	void debugDraw();
	Octree& getNodeById(NodeId id);

private:
	OctreeNode* root;
	NodeId nodeCounter = 0;
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
 - Every octree node that is not a leaf, has an empty colliders list.
 - Leaf nodes cannot have more than LEAF_THRESHOLD # of colliders, UNLESS
   the maximum recursion depth has been exceeded.
 
 */
class OctreeNode {
public:
	friend Octree;

	OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth);
	~OctreeNode();

	void insert(BoxCollider&);
	CollisionInfo intersects(BoxCollider&);
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

private:
	std::vector<GameObject*> collidees;
};