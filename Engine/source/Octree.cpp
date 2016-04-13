#include "Collision.h"

Octree::Octree() {

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

CollisionInfo Octree::raycast(const Ray&) {
	return CollisionInfo(); // TODO: placeholder
};

CollisionInfo Octree::collidesWith(const BoxCollider&) {
	return CollisionInfo(); // TODO: placeholder
};

void Octree::debugDraw() {
	if (root) {
		root->debugDraw();
	}
}