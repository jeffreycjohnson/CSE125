#include "Collision.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "Renderer.h"
#include "Timer.h"
#include <stack>
#include <iostream>

// Raycasting constants
const float Octree::RAY_MIN = FLT_EPSILON;
const float Octree::RAY_MAX = FLT_MAX;
const float Octree::RAY_STEP = 0.01f;

//Octree* Octree::STATIC_TREE  = nullptr; // Globals are bad.
//Octree* Octree::DYNAMIC_TREE = nullptr;

Octree::Octree(glm::vec3 min, glm::vec3 max) {
	root = new OctreeNode(min, max, this);
	objects = 0;
};

Octree::~Octree() {
	delete root;
};

void Octree::addNode(NodeId node, OctreeNode* self) {
	if (self == nullptr) {
		LOG("Trying to add null OctreeNode, with nodeID: " + std::to_string(node));
	}
	nodeMap[node] = self;
};

void Octree::removeNode(NodeId node) {
	auto iter = nodeMap.find(node);
	if (iter != nodeMap.end()) {
		nodeMap.erase(iter); // Remove the node
	}
	else {
		LOG("Tried to remove nonexisting OctreeNode, with nodeID: " + std::to_string(node));
	}
};

void Octree::insert(Collider* obj) {
	if (root && obj != nullptr) {
		if (obj->nodeId == UNKNOWN_NODE) {
			if (obj->passive && (restriction == STATIC_ONLY || restriction == BOTH)) {
				root->insert(obj, obj->getAABB());
				objects++;
			}
			else if (!obj->passive && (restriction == DYNAMIC_ONLY || restriction == BOTH)) {
				root->insert(obj, obj->getAABB());
				objects++;
			}
		}
	}
}

void Octree::remove(Collider* obj) {
	if (root && obj != nullptr) {
		OctreeNode* node = nodeMap[obj->nodeId];
		node->remove(obj); // Skip having to search through the whole entire tree!
		--objects;
	}
}

void Octree::build(BuildMode mode, const GameObject& root) {

	// THIS IS EXPENSIVE, DON'T DO THIS A LOT!!!!
	// TODO: Implement some kind of clear method for the octree
	
	long objCounter = 0;
	double startTime = Timer::time();
	std::stack<Transform> stack;
	stack.push(root.transform);
	restriction = mode;

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

	//LOG("Created Octree with { " + std::to_string(objCounter) + " } colliders.");
	std::cerr << "Created Octree with: " << objCounter << " colliders." << std::endl;
	std::cerr << "(Time taken: " << Timer::time() - startTime << " ms)" << std::endl;

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

std::unordered_map<NodeId, OctreeNode*>::iterator Octree::begin() {
	return nodeMap.begin();
}

std::unordered_map<NodeId, OctreeNode*>::iterator Octree::end() {
	return nodeMap.end();
}

void Octree::rebuild()
{
	if (root != nullptr) {
		std::list<Collider*> colliders; // TODO: maybe vector would be faster, not sure
		for (auto pair : nodeMap) {
			auto node = pair.second;
			for (auto colliderPtr : node->colliders) {
				colliders.push_back(colliderPtr);
			}
		}
		for (auto collider : colliders) {
			remove(collider);
		}
		for (auto collider : colliders) {
			root->insert(collider, collider->getAABB());
		}
	}
}

RayHitInfo Octree::raycast(const Ray & ray)
{
	RayHitInfo hitInfo;
	if (root) {
		root->raycast(ray, hitInfo);
	}
	return hitInfo;
}

CollisionInfo Octree::collidesWith(Collider* ptr) { // TODO: There is either a bug here, or in OctreeNode::collidesWith
	CollisionInfo colInfo;
	colInfo.collider = ptr;
	if (root) {
		BoxCollider* box = dynamic_cast<BoxCollider*>(ptr);
		SphereCollider* sphere = dynamic_cast<SphereCollider*>(ptr);
		CapsuleCollider* capsule = dynamic_cast<CapsuleCollider*>(ptr);

		if (box != nullptr) {
			return root->collidesWith(*box, colInfo);
		}
		else if (sphere != nullptr) {
			return root->collidesWith(*sphere, colInfo); // TODO: remember to update sphere & capsule coolideswith() with changes to Box version
		}
		else if (capsule != nullptr) {
			return root->collidesWith(*capsule, colInfo);
		}

	}
	else {
		return colInfo;
	}
};

void Octree::debugDraw() {
	if (root) {
		root->debugDraw();
	}
}