#include "Collision.h"
#include "Renderer.h"
#include "BoxCollider.h"

OctreeNode::OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth) {
	this->min = min;
	this->max = max;
	this->tree = tree;
	this->parent = parent;
	this->nodeId = ++tree->nodeCounter; // the pre-increment is important here
	this->depth = depth;
	tree->addNode(this->nodeId, this);
}

OctreeNode::~OctreeNode() {
	// Alert the tree that we're going away now
	for (auto child : children) {
		if (child != nullptr) {
			delete child;
		}
	}
	tree->removeNode(this->nodeId);
}

CollisionInfo OctreeNode::raycast(const Ray& ray) {
	return CollisionInfo(); // TODO: implement octree node raycast
};
CollisionInfo OctreeNode::collidesWith(const BoxCollider& box) {

	// Check object against all of the objects in our colliders list

	// If we have children, check them afterwards

	return CollisionInfo(); // TODO: implement API for collidesWith for octree
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

bool OctreeNode::insert(BoxCollider& box) {
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
			return false;
		}
		else if (collisions == 1) {
			// Insert our box into the chosen child.
			return children[mask]->insert(box);
		}
		else {
			// Node successfully inserted into this node
			box.nodeId = nodeId;
			colliders.push_back(&box);

			// If we have too many
			if (colliders.size() > Octree::LEAF_THRESHOLD && depth < Octree::MAX_DEPTH) {
				subdivide();
			}
			return true;
		}
	}
}

void OctreeNode::subdivide() {
	if (children.empty() && depth < Octree::MAX_DEPTH) {
		// Figure out hte dimensions of each child
		glm::vec3 dims = max - min;
		float xDist = dims.x / 2;
		float yDist = dims.y / 2;
		float zDist = dims.z / 2;

		// We need to generate 6 new "min" points, and 6 new "max" points & the center
		glm::vec3 dist = glm::vec3(xDist, yDist, zDist);
		glm::vec3 center = min + dist;

		glm::vec3 min1(min.x, min.y + yDist, min.z); // I know the names are bad, but I drew a diagram.
		glm::vec3 min2(min.x + xDist, min.y + yDist, min.z);
		glm::vec3 min3(min.x + xDist, min.y, min.z);

		glm::vec3 min4(min.x, min.y, min.z + zDist);
		glm::vec3 min5(min.x, min.y + yDist, min.z + zDist);
		glm::vec3 min6(min.x + xDist, min.y, min.z + zDist);

		// [max points]   (should map 1-to-1 with min points.)
		glm::vec3 max1 = min1 + dist;
		glm::vec3 max2 = min2 + dist;
		glm::vec3 max3 = min3 + dist;
		glm::vec3 max4 = min4 + dist;
		glm::vec3 max5 = min5 + dist;
		glm::vec3 max6 = min6 + dist;

		// Create children...
		children.push_back(new OctreeNode(min, center, tree, this, depth + 1));
		children.push_back(new OctreeNode(center, max, tree, this, depth + 1));

		children.push_back(new OctreeNode(min1, max1, tree, this, depth + 1));
		children.push_back(new OctreeNode(min2, max2, tree, this, depth + 1));
		children.push_back(new OctreeNode(min3, max3, tree, this, depth + 1));
		children.push_back(new OctreeNode(min4, max4, tree, this, depth + 1));
		children.push_back(new OctreeNode(min5, max5, tree, this, depth + 1));
		children.push_back(new OctreeNode(min6, max6, tree, this, depth + 1));

		// Insert all of our colliders into each of those children, and let recursion deal with it
		std::vector<BoxCollider*> stragglers;
		for (auto bbox : colliders) {
			for (auto child : children) {
				if (!child->insert(*bbox)) {
					// If we couldn't insert this guy further down the tree, keep it here
					stragglers.push_back(bbox);
				}
			}
		}

		// Clear the colliders vector and insert remaining stragglers
		colliders.clear();
		colliders = stragglers;
	
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