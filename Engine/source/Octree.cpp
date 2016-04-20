#include "Collision.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "Renderer.h"
#include <stack>

// Raycasting constants
static const float RAY_MIN = FLT_EPSILON;
static const float RAY_MAX = FLT_MAX;
static const float RAY_STEP = 0.01f;

Octree* Octree::STATIC_TREE  = nullptr;
Octree* Octree::DYNAMIC_TREE = nullptr;

Octree::Octree(glm::vec3 min, glm::vec3 max) {
	root = new OctreeNode(min, max, this);
};

Octree::~Octree() {

};

void Octree::addNode(NodeId node, OctreeNode* self) {
	nodeMap[node] = self;
};

void Octree::removeNode(NodeId node) {
	auto iter = nodeMap.find(node);
	if (iter != nodeMap.end()) {
		nodeMap.erase(iter); // Remove the node
	}
};

void Octree::insert(Collider* obj) {
	if (root)
		root->insert(obj, obj->getAABB());
}

void Octree::remove(Collider* obj) {
	if (root) {
		OctreeNode* node = nodeMap[obj->nodeId];
		node->remove(obj); // Skip having to search through the whole entire tree!
	}
}

void Octree::build(BuildMode mode, const GameObject& root) {

	// THIS IS EXPENSIVE, DON'T DO THIS A LOT!!!!
	
	long objCounter = 0;
	std::stack<Transform> stack;
	stack.push(root.transform);

	// Traverse through the transform hierarchy (DFS)
	while (!stack.empty()) {

		// Get gameObject from the Transform
		GameObject* current = stack.top().gameObject;
		stack.pop();

		if (current == nullptr) {
			break; // Don't panic
		}

		// REMEMBER: Currently a game object can only have unique components.
		//           If this ever changes, this code is 100% broken.
		// Get colliders & insert them
		BoxCollider* box         = current->getComponent<BoxCollider>();
		SphereCollider* sphere   = current->getComponent<SphereCollider>();
		CapsuleCollider* capsule = current->getComponent<CapsuleCollider>();

		if (box != nullptr)  {
			this->insert(box);
			++objCounter;
		}
		if (sphere != nullptr) {
			this->insert(sphere);
			++objCounter;
		}
		if (capsule != nullptr) {
			this->insert(capsule);
			++objCounter;
		}

		// Get the transform's children
		Transform t = current->transform;
		for (auto childPtr : t.children) {
			stack.push(*childPtr);
		}
	}

	LOG(objCounter); // TODO: remove debug log later

}

OctreeNode* Octree::getNodeById(NodeId node) {
	auto iter = nodeMap.find(node);
	if (iter != nodeMap.end()) {
		return nodeMap[node];
	}
	else {
		return nullptr;
	}
}

CollisionInfo Octree::raycast(Ray ray, float min, float max, float step) {
	if (root && min < max) {
		CollisionInfo colInfo;
		int steps = std::ceil((max - min) / step);
		for (int i = 0; i < steps; ++i) {
			ray.t = i * step + min;
			colInfo = root->raycast(ray);
			if (colInfo.collisionOccurred) {
				return colInfo; // Return earliest collision
			}
		}
		return colInfo;
	}
	else {
		return CollisionInfo();
	}
};

CollisionInfo Octree::collidesWith(const BoxCollider& box) {
	if (root) {
		return root->collidesWith(box);
	}
	else {
		return CollisionInfo();
	}
};

void Octree::debugDraw() {
	if (root) {
		root->debugDraw();
	}
}