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

};

CollisionInfo Octree::intersects(const BoxCollider&) {

};