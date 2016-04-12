#include "Collision.h"

OctreeNode::OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth) {
	this->min = min;
	this->max = max;
	this->tree = tree;
	this->parent = parent;
	this->nodeId = tree->nodeCounter++;
	this->depth = depth + 1;
	tree->addNode(this->nodeId, this);
}

OctreeNode::~OctreeNode() {
	// Alert the tree that we're going away now
	tree->removeNode(this->nodeId);
}

CollisionInfo OctreeNode::raycast(const Ray&) {

};
CollisionInfo OctreeNode::intersects(const BoxCollider&) {

};

void OctreeNode::subdivide() {
	if (children.empty() && depth < Octree::MAX_DEPTH) {
		// Create children...

		// Do maths to find their bounding boxes **snoooore**

		// Insert all of our colliders into each of those children, and let recursion deal with it
		for (auto bbox : colliders) {
			for (auto child : children) {
				child->insert(*bbox); // TODO: Maybe using a reference in inserts() isn't such a good idea...
			}
		}

		// Clear the colliders vector
		colliders.clear();
	
	}
	//else // TODO: Will I need to do anything in the else case should a subdivide() call fail?
}

bool OctreeNode::isLeaf() const {
	// An octree node with no children must be a leaf
	return children.empty();
}