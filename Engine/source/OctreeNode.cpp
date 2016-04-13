#include "Collision.h"
#include "Renderer.h"
#include "BoxCollider.h"

OctreeNode::OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth) {
	this->min = min;
	this->max = max;
	this->tree = tree;
	this->parent = parent;
	this->nodeId = ++tree->nodeCounter; // the pre-increment is important here
	this->depth = depth + 1;
	tree->addNode(this->nodeId, this);
}

OctreeNode::~OctreeNode() {
	// Alert the tree that we're going away now
	tree->removeNode(this->nodeId);
}

CollisionInfo OctreeNode::raycast(const Ray& ray) {
	return CollisionInfo(); // TODO: placeholder
};
CollisionInfo OctreeNode::collidesWith(const BoxCollider& box) {
	return CollisionInfo(); // TODO: placeholder
};

bool OctreeNode::intersects(const BoxCollider& box) {
	float width = std::abs(max.x - min.x);
	float height = std::abs(max.y - min.y);
	float depth = std::abs(max.z - min.z);
	glm::vec3 center(width / 2 + min.x, height / 2 + min.y, depth / 2 + min.z);
	glm::vec3 dims(width, height, depth);
	BoxCollider bounds(center, dims);
	return bounds.intersects(box);
}

void OctreeNode::insert(BoxCollider& box) {
	// Keeps track of the number of intersections the box has with our children
	int collisions = 0;
	unsigned int index = 0;
	
	// If this node already has children, recurse
	if (!isLeaf()) {
		// Saves which children have registered collisions with the box
		// e.g. |1|1|1|0|0|0|1|0|  -> child 1, 5, 6, 7 have collisions
		unsigned char indices = 0;
		unsigned char mask = 0x00;
		for (auto child : children) {
			if (child->intersects(box)) {
				mask &= index;
			}
			++index;
		}
		if (collisions == 0) {
			// If for some reason this box does not collide with any of our children,
			// it therefore cannot collide with this node. So if we get, here... wtf!?
			box.nodeId = Octree::UNKNOWN_NODE;
		}
		else if (collisions == 1) {
			// Insert our box into the chosen child.
			return children[mask]->insert(box);
		}
		else {
			// NOTE: I wanted to pass in box as a const reference, but the compiler isn't
			box.nodeId = nodeId;
			colliders.push_back(&box);
		}
	}
}

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

void OctreeNode::debugDraw() {
	// Only draw octree leaves that have objects
	if (children.empty() && !colliders.empty()) {
		glm::vec3 center = (max - min); // TODO: Our debug drawing should probably be more descriptive & configurable
		center /= 2;
		glm::vec3 scale(1, 1, 1);
		glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
		Renderer::drawBox(center, scale, color);
	}
}