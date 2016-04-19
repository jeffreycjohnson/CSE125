#include "Collision.h"
#include "Renderer.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

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
	CollisionInfo colInfo;
	glm::vec3 point = ray.getCurrentPosition();

	for (auto obj : colliders) {
		if (obj->insideOrIntersects(point)) {
			colInfo.add(obj);
		}
	}

	if (isLeaf()) {
		return colInfo;
	}
	
	// TODO: Figure out which octant the point is in & recurse only there (optimization)

	for (auto child : children) {
		if (child != nullptr)
			colInfo.merge(child->raycast(ray));
	}

	return colInfo;
};

CollisionInfo OctreeNode::collidesWith(const BoxCollider& box) {

	// TODO: Probably refactor this header to take a Collider* or use dynamic_cast<> (?)
	CollisionInfo info;

	// Check object against all of the objects in our colliders list
	for (auto colliderPtr : colliders) {
		switch (colliderPtr->getColliderType()) {

			case ColliderType::AABB:
			{
			   BoxCollider* myBox = (BoxCollider*)colliderPtr;
			   if (myBox->intersects(box)) {
				   info.add(myBox);
			   }
			   break;
			}
			case ColliderType::SPHERE:
			{
				SphereCollider* mySphere = (SphereCollider*)colliderPtr;
				if (mySphere->intersects(box)) {
					info.add(mySphere);
				}
				break;
			}
			case ColliderType::CAPSULE:
			{
				CapsuleCollider* myCapsule = (CapsuleCollider*)colliderPtr;
				if (myCapsule->intersects(box)) {
					info.add(myCapsule);
				}
				break;
			}
		}
	}

	// If we have children, check them afterwards
	for (auto child : children) {
		info.merge(child->collidesWith(box));
	}

	return info;

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

bool OctreeNode::insert(Collider* colliderBeingInserted, const BoxCollider& box) {
	
	// Keeps track of the number of intersections the box has with our children
	int collisions = 0;
	unsigned int index = 0;
	
	// If this node already has children, recurse
	if (!isLeaf()) {
	
		// <indicies> saves which children have registered collisions with the box
		// e.g.             |1|1|1|0|0|0|1|0|  -> child 1, 5, 6, 7 have collisions
		unsigned char indices = 0;
		unsigned char mask = 0x00;
		
		for (auto child : children) {	
			// Despite what kind of collider we are inserting, we always want
			// to do intersection testing with axis-aligned bounding boxes, b/c they are cheap
			if (child->intersects(box)) {
				mask &= index;
			}
			++index;
		}
		
		if (collisions == 0) {
			// If for some reason this box does not collide with any of our children,
			// it therefore cannot collide with this node. So if we get, here... wtf!?
			colliderBeingInserted->nodeId = Octree::UNKNOWN_NODE;
			return false;
		}
		else if (collisions == 1) {
			// Insert our box into the chosen child.
			return children[mask]->insert(colliderBeingInserted, box);
		}
		else {
			// Node successfully inserted into this node
			colliderBeingInserted->nodeId = nodeId;
			colliders.push_back(colliderBeingInserted);

			// If we have too many
			if (colliders.size() > Octree::LEAF_THRESHOLD && depth < Octree::MAX_DEPTH) {
				subdivide();
			}
			return true;
		}
	}
}

void OctreeNode::remove(Collider * colliderBeingRemoved)
{
	for (auto iter = colliders.begin(); iter != colliders.end(); ++iter) {
		// TODO: Double check that this will function properly
		if (*iter == colliderBeingRemoved) {
			colliders.erase(iter, iter + 1);
		}
	}
}

void OctreeNode::subdivide() {
	if (children.empty() && depth < Octree::MAX_DEPTH) {
		// Figure out the dimensions of each child
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
		std::vector<Collider*> stragglers;
		for (auto collider : colliders) {
			for (auto child : children) {
				// Aaaaand this method becomes MORE expensive b/c we have to compute the AABBs again...
				if (!child->insert(collider, collider->getAABB())) {
					// If we couldn't insert this guy further down the tree, keep it here
					stragglers.push_back(collider);
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
	if (true){//children.empty() && !colliders.empty()) {
		glm::vec3 center = (max - min); // TODO: Our debug drawing should probably be more descriptive & configurable
		center /= 2;
		glm::vec3 scale(1, 1, 1);
		glm::vec4 color(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
		Renderer::drawBox(center, scale, color);
	}
}