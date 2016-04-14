#include "Collision.h"
#include "Collider.h"

CollisionInfo::CollisionInfo() {
	collisionOccurred = false; // By default, no collision has been detected
	numCollisions = 0;
};

CollisionInfo::~CollisionInfo() {

};


void CollisionInfo::merge(const CollisionInfo& other) {
	numCollisions += other.numCollisions;
	collisionOccurred = collisionOccurred || other.collisionOccurred;
	for (auto collider : other.collidees) {
		collidees.insert(collider);
	}
}

void CollisionInfo::add(Collider* object) {
	if (object == nullptr) {
		return;
	}
	numCollisions++;
	collisionOccurred = true;
	collidees.insert(object->gameObject);
}