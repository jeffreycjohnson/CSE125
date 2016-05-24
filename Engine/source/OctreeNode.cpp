#include "Collision.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include <sstream>
#include <iostream>
#include <string>         // std::string

OctreeNode::OctreeNode(glm::vec3 min, glm::vec3 max, Octree* tree, OctreeNode* parent, int depth) {
	this->min = min;
	this->max = max;
	this->tree = tree;
	this->parent = parent;
	this->nodeId = ++tree->nodeCounter; // the pre-increment is important here
	this->depth = depth;
	tree->addNode(this->nodeId, this);

	// Precompute axis-aligned bounding box for this Octree node
	float xDiameter = std::abs(max.x - min.x);
	float yDiameter = std::abs(max.y - min.y);
	float zDiameter = std::abs(max.z - min.z);
	glm::vec3 center = glm::vec3(max.x - xDiameter / 2, max.y - yDiameter / 2, max.z - zDiameter / 2);
	glm::vec3 dims(xDiameter / 2, yDiameter / 2, zDiameter / 2); // min points didn't match

	myAABB = new BoxCollider(center, dims);
	myAABB->setMinAndMax(min, max);
	myAABB->setAxisAligned(true);

}

OctreeNode::~OctreeNode() {
	// Alert the tree that we're going away now
	for (auto child : children) {
		if (child != nullptr) {
			delete child;
		}
	}
	delete myAABB;
	tree->removeNode(this->nodeId);
}

void OctreeNode::raycast(const Ray& ray, RayHitInfo& hitInfo, const std::initializer_list<Collider*>& ignore) {

	RayHitInfo againstMe = myAABB->raycast(ray);

	// If something has become NaN, we should reset it
	if (std::isnan(hitInfo.hitTime)) {
		hitInfo.hitTime = Octree::RAY_MAX;
	}

	if (againstMe.hitTime > hitInfo.hitTime || hitInfo.hitTime <= 0 ) {
		return; // Nothing inside of us will collide with this
	}

	for (auto obj : colliders) {
		bool skip = false;
		for (auto ignored : ignore) {
			if (obj == ignored) {
				skip = true; // Skip ignored or inactive colliders
			}
		}
		if (skip || !obj->active) continue;
		if (obj->gameObject) {
			if (!obj->gameObject->getActive()) {
				continue;
			}
		}
		if (obj->gameObject->getName().find("_trigger") != std::string::npos) {
			continue;
		}
		auto temphit = obj->raycast(ray);
		if (temphit.intersects /*&& temphit.hitTime > 0*/) {
			if (temphit.hitTime < hitInfo.hitTime) {
				hitInfo = temphit;
			}
		}
	}

	for (auto child : children) {
		if (child != nullptr) {
			child->raycast(ray, hitInfo, ignore);
		}
	}

};

CollisionInfo OctreeNode::collidesWith(Collider* collider, const BoxCollider& aabb, CollisionInfo& info) {

	// Check object against all of the objects in our colliders list
	if (intersects(aabb)) {
		for (auto colliderPtr : colliders) {
			if (colliderPtr == collider) continue; // Don't check colliders against themselves
			if (!colliderPtr->active)    continue; // Skip inactive colliders
			switch (colliderPtr->getColliderType()) {

				case ColliderType::BOX:
				{
					BoxCollider* myBox = (BoxCollider*)colliderPtr;
					if (myBox->intersects(*(BoxCollider*)collider)) {
						info.add(myBox);
					}
					break;
				}
				case ColliderType::SPHERE:
				{
					SphereCollider* mySphere = (SphereCollider*)colliderPtr;
					if (mySphere->intersects(*(SphereCollider*)collider)) {
						info.add(mySphere);
					}
					break;
				}
				case ColliderType::CAPSULE:
				{
					CapsuleCollider* myCapsule = (CapsuleCollider*)colliderPtr;
					if (myCapsule->intersects(*(CapsuleCollider*)collider)) {
						info.add(myCapsule);
					}
					break;
				}
			}
		}
		
		// If we have children, check them afterwards
		for (auto child : children) {
			if (child->colliders.size() > 0 && child->isLeaf())
				child->collidesWith(collider, aabb, info);
		}
	}

	return info;

};

bool OctreeNode::intersects(const BoxCollider& box) {

	return myAABB->intersects(box);

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
			colliderBeingInserted->octree = this->tree;
			colliders.push_back(colliderBeingInserted);
			return true;
		}
	}
	else {
		// If we are a leaf node
		if (colliders.size() >= Octree::LEAF_THRESHOLD && depth < Octree::MAX_DEPTH) {
			// The subdivide *may* fail, so for now we recurse
			bool success = subdivide();
			if (!success) {
				colliders.push_back(colliderBeingInserted);
				colliderBeingInserted->nodeId = nodeId;
				colliderBeingInserted->octree = this->tree;
				return true;
			}
		}
		else {
			colliders.push_back(colliderBeingInserted);
			colliderBeingInserted->nodeId = nodeId;
			colliderBeingInserted->octree = this->tree;
			return true;
		}
	}
	return false;
}

void OctreeNode::remove(Collider * colliderBeingRemoved)
{
	if (colliderBeingRemoved != nullptr) {
		colliderBeingRemoved->nodeId = Octree::UNKNOWN_NODE;
		colliderBeingRemoved->octree = nullptr;
		colliders.remove(colliderBeingRemoved);
	}
}

bool OctreeNode::subdivide() {
	if (children.empty() && depth < Octree::MAX_DEPTH) {
		// Figure out the dimensions of each child
		float xDist = std::abs(max.x - min.x) / 2;
		float yDist = std::abs(max.y - min.y) / 2;
		float zDist = std::abs(max.z - min.z) / 2;

		// We need to generate 6 new "min" points, and 6 new "max" points & the center
		glm::vec3 dist(xDist, yDist, zDist);

		glm::vec3 center( (max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2);
	
		// Generate the first 2 (trivial) children
		children.push_back(new OctreeNode(this->min, center, tree, this, depth + 1));
		children.push_back(new OctreeNode(center, this->max, tree, this, depth + 1));

		// [min points]
		glm::vec3 min1(min.x,         min.y + yDist, min.z); // I know the names are bad, but I drew a diagram.
		glm::vec3 min2(min.x + xDist, min.y + yDist, min.z);
		glm::vec3 min3(min.x + xDist, min.y,         min.z);

		glm::vec3 min4(min.x,         min.y,         min.z + zDist);
		glm::vec3 min5(min.x,         min.y + yDist, min.z + zDist);
		glm::vec3 min6(min.x + xDist, min.y,         min.z + zDist);

		// [max points]   (should map 1-to-1 with min points.)
		glm::vec3 max1 = min1 + dist;
		glm::vec3 max2 = min2 + dist;
		glm::vec3 max3 = min3 + dist;
		glm::vec3 max4 = min4 + dist;
		glm::vec3 max5 = min5 + dist;
		glm::vec3 max6 = min6 + dist;

		// Create children...
		children.push_back(new OctreeNode(min1, max1, tree, this, depth + 1));
		children.push_back(new OctreeNode(min2, max2, tree, this, depth + 1));
		children.push_back(new OctreeNode(min3, max3, tree, this, depth + 1));
		children.push_back(new OctreeNode(min4, max4, tree, this, depth + 1));
		children.push_back(new OctreeNode(min5, max5, tree, this, depth + 1));
		children.push_back(new OctreeNode(min6, max6, tree, this, depth + 1));

		// Insert all of our colliders into each of those children, and let recursion deal with it
		std::list<Collider*> stragglers;
		for (auto collider : colliders) {

			BoxCollider aabb = collider->getAABB();
			OctreeNode* favoriteChild = nullptr;
			int childIntersections = 0;
			for (auto child : children) {
				
				// Aaaaand this method becomes MORE expensive b/c we have to compute the AABBs again...
				
				if (child->myAABB->intersects(aabb)) {
					childIntersections++;
					favoriteChild = child;
				}

			}

			if (childIntersections == 1 && favoriteChild != nullptr) {
				favoriteChild->insert(collider, aabb);
			}
			else {
				stragglers.push_back(collider);
			}
		}

		// Clear the colliders vector and insert remaining stragglers
		colliders.clear();
		colliders = stragglers;

		return true;

	}
	else {
		return false; // Subdivide failed
	}
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

std::list<Collider*>::iterator OctreeNode::begin() {
	return colliders.begin();
};
std::list<Collider*>::iterator OctreeNode::end() {
	return colliders.end();
};

void OctreeNode::debugDraw() {
	// Only draw octree leaves that have objects
	float xDiameter = std::abs(max.x - min.x);
	float yDiameter = std::abs(max.y - min.y);
	float zDiameter = std::abs(max.z - min.z);
	glm::vec3 center = glm::vec3(min.x + (xDiameter / 2), min.y + (yDiameter / 2), min.z + (zDiameter / 2));
	glm::vec3 scale(xDiameter / 2, yDiameter / 2, zDiameter / 2);
	glm::vec4 color = glm::vec4(DebugPass::octreeColor, 1);

	//--- Verifying boxes are drawing properly ---
	//BoxCollider bounds(center, scale);
	//bounds.debugDraw();   // Sanity check
	//myAABB->debugDraw();  // Sanity check
	//--------------------------------------------

	// Draw spheres @ min & max corners
	Renderer::drawSphere(min, 0.25f, glm::vec4(1, 0, 0, 1)); // min points RED
	Renderer::drawSphere(max, 0.35f, glm::vec4(0, 1, 0, 1)); // max points GREEN

	// Only render nodes with colliders in them and/or the root
	if (colliders.size() > 0 || this == tree->root) {
		Renderer::drawBox(center, scale, color);
	}
	for (auto child : children) {
		child->debugDraw();
	}
}