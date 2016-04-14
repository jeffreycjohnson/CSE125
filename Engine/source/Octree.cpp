#include "Collision.h"

// Raycasting constants
static const float RAY_MIN = FLT_EPSILON;
static const float RAY_MAX = FLT_MAX;
static const float RAY_STEP = 0.01f;

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

void Octree::insert(BoxCollider& box) {
	if (root)
		root->insert(box);
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