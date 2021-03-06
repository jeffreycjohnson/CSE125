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
const float Octree::RAY_MIN = 0.0f;
const float Octree::RAY_MAX = FLT_MAX;

Octree::Octree(glm::vec3 min, glm::vec3 max) {
	objects = 0;
	nodeCounter = 0;
	restriction = BuildMode::BOTH;
	root = new OctreeNode(min, max, this);
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
		if (node != nullptr) {
			node->remove(obj); // Skip having to search through the whole entire tree!
			--objects;
		}
		else {
			auto str = "Error! Trying to remove collider from nonexisting OctreeNode with ID: " + std::to_string(obj->nodeId);
			LOG(str);
		}
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
			if (node != nullptr) {
				for (auto colliderPtr : node->colliders) {
					colliders.push_back(colliderPtr);
				}
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

RayHitInfo Octree::raycast(const Ray & ray, float t_min, float t_max, const std::initializer_list<Collider*>& ignore, bool ignoreTriggers)
{

	RayHitInfo hitInfo;
	if (NetworkManager::getState() == NetworkState::CLIENT_MODE) return hitInfo;

	if (root) {
		root->raycast(ray, hitInfo, ignore, ignoreTriggers);
	}
	if (hitInfo.hitTime < t_min || hitInfo.hitTime > t_max) {
		hitInfo.intersects = false;
	}
	return hitInfo;
}

CollisionInfo Octree::collidesWith(Collider* ptr) { // TODO: There is either a bug here, or in OctreeNode::collidesWith

	CollisionInfo colInfo;
	if (NetworkManager::getState() == NetworkState::CLIENT_MODE) return colInfo;

	colInfo.collider = ptr;

	if (root && ptr != nullptr) {
		BoxCollider aabb = ptr->getAABB(); // Avoid having to recompute this over and over and over...
		if (ptr->active) {
			return root->collidesWith(ptr, aabb, colInfo);
		}
		return colInfo;
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