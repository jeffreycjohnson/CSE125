#include "Collision.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include <sstream>

OctreeNode::OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth) {
	this->min = min;
	this->max = max;
	this->tree = tree;
	this->parent = parent;
	this->nodeId = ++tree->nodeCounter; // the pre-increment is important here
	this->depth = depth;
	tree->addNode(this->nodeId, this);
	//LOG(this->toString());
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

// TODO: Condense the "collidesWith" methods into one method, since there's a lot of repetition
CollisionInfo OctreeNode::collidesWith(const BoxCollider& box) {

	CollisionInfo info;
	info.collider = (Collider*)&box;

	// Check object against all of the objects in our colliders list
	//if (intersects(box)) {
		for (auto colliderPtr : colliders) {
			if (colliderPtr == &box) continue; // Don't check colliders against themselves
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
	//}

	return info;

};

CollisionInfo OctreeNode::collidesWith(const SphereCollider& collider) {

	CollisionInfo info;
	info.collider = (Collider*)&collider;

	// Check object against all of the objects in our colliders list
	for (auto colliderPtr : colliders) {
		if (colliderPtr == &collider) continue; // Don't check colliders against themselves
		switch (colliderPtr->getColliderType()) {
			case ColliderType::AABB:
			{
				BoxCollider* myBox = (BoxCollider*)colliderPtr;
				if (myBox->intersects(collider)) {
					info.add(myBox);
				}
				break;
			}
			case ColliderType::SPHERE:
			{
				SphereCollider* mySphere = (SphereCollider*)colliderPtr;
				if (mySphere->intersects(collider)) {
					info.add(mySphere);
				}
				break;
			}
			case ColliderType::CAPSULE:
			{
				CapsuleCollider* myCapsule = (CapsuleCollider*)colliderPtr;
				if (myCapsule->intersects(collider)) {
					info.add(myCapsule);
				}
				break;
			}
		}
	}

	// If we have children, check them afterwards
	for (auto child : children) {
		info.merge(child->collidesWith(collider));
	}

	return info;

};

CollisionInfo OctreeNode::collidesWith(const CapsuleCollider& collider) {

	CollisionInfo info;
	info.collider = (Collider*)&collider;

	// Check object against all of the objects in our colliders list
	for (auto colliderPtr : colliders) {
		if (colliderPtr == &collider) continue; // Don't check colliders against themselves
		switch (colliderPtr->getColliderType()) {
			case ColliderType::AABB:
			{
				BoxCollider* myBox = (BoxCollider*)colliderPtr;
				if (myBox->intersects(collider)) {
					info.add(myBox);
				}
				break;
			}
			case ColliderType::SPHERE:
			{
				SphereCollider* mySphere = (SphereCollider*)colliderPtr;
				if (mySphere->intersects(collider)) {
					info.add(mySphere);
				}
				break;
			}
			case ColliderType::CAPSULE:
			{
				CapsuleCollider* myCapsule = (CapsuleCollider*)colliderPtr;
				if (myCapsule->intersects(collider)) {
					info.add(myCapsule);
				}
				break;
			}
		}
	}

	// If we have children, check them afterwards
	for (auto child : children) {
		info.merge(child->collidesWith(collider));
	}

	return info;

};

bool OctreeNode::intersects(const BoxCollider& box) {
	float xDiameter = std::abs(max.x - min.x);
	float yDiameter = std::abs(max.y - min.y);
	float zDiameter = std::abs(max.z - min.z);
	glm::vec3 center = glm::vec3(max.x - xDiameter / 2, max.y - yDiameter / 2, max.z - zDiameter / 2);
	glm::vec3 dims(xDiameter / 2, yDiameter / 2, zDiameter / 2);
	BoxCollider bounds(center, dims);
	return bounds.intersects(box);
}

bool OctreeNode::insert(Collider* colliderBeingInserted, const BoxCollider& colliderAABB) {
	
	// Keeps track of the number of intersections the box has with our children
	int collisions = 0;
	unsigned int index = 0;
	
	if (!isLeaf()) {
	
		// If this is an internal node, attempt to place the desired object into
		// the child nodes. If there are multiple collisions between the object
		// we are inserting & our immediate children, the object is too large,
		// or in an awkward position, and must rest inside of this internal node.

		int lastCollision = 0;

		for (auto child : children) {	
			// Despite what kind of collider we are inserting, we always want
			// to do intersection testing with axis-aligned bounding boxes, b/c they are cheap
			if (collisions > 1) break; // optimization
			if (child->intersects(colliderAABB)) {
				lastCollision = index;
				++collisions;
			}
			++index;
		}
		
		if (collisions == 1) {
			// Insert our box into the chosen child.
			return children[lastCollision]->insert(colliderBeingInserted, colliderAABB);
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
	else {
		// If we are a leaf node
		if (depth < Octree::MAX_DEPTH) {
			subdivide(); // The subdivide *may* fail, so for now we recurse
			insert(colliderBeingInserted, colliderAABB);
		}
		else {
			colliders.push_back(colliderBeingInserted);
		}
	}
}

void OctreeNode::remove(Collider * colliderBeingRemoved)
{
	for (auto iter = colliders.begin(); iter != colliders.end(); iter++) {
		// TODO: Double check that this will function properly
		if (*iter == colliderBeingRemoved) {
			colliders.erase(iter, iter + 1);
			break;
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

		LOG(this->toString());
	
	}
	//else // TODO: Will I need to do anything in the else case should a subdivide() call fail?
}

bool OctreeNode::isLeaf() const {
	// An octree node with no children must be a leaf
	return children.empty();
}

std::string OctreeNode::toString() const
{
	std::stringstream str;
	str << "Min: [" << min.x << "," << min.y << "," << min.z << "]" << std::endl;
	str << "Max: [" << max.x << "," << max.y << "," << max.z << "]" << std::endl;
	str << "Colliders: " << colliders.size() << std::endl;
	str << "Children: " << children.size();
	return str.str();
}

std::vector<Collider*>::iterator OctreeNode::begin() {
	return colliders.begin();
};
std::vector<Collider*>::iterator OctreeNode::end() {
	return colliders.end();
};

void OctreeNode::debugDraw() {
	// Only draw octree leaves that have objects
	float xDiameter = std::abs(max.x - min.x);
	float yDiameter = std::abs(max.y - min.y);
	float zDiameter = std::abs(max.z - min.z);
	glm::vec3 center = glm::vec3(max.x - xDiameter / 2, max.y - yDiameter / 2, max.z - zDiameter / 2);
	glm::vec3 scale(xDiameter / 2, yDiameter / 2, zDiameter / 2);
	glm::vec4 color = glm::vec4(DebugPass::octreeColor, 1);
	
	// Only render nodes with colliders in them and/or the root
	if (colliders.size() > 0 || this == tree->root)
		Renderer::drawBox(center, scale, color); 
	for (auto child : children) {
		child->debugDraw();
	}
}