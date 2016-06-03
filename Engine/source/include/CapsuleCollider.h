#pragma once
#include "ForwardDecs.h"
#include "Collider.h"

class CapsuleCollider : public Collider
{
private:

	// A capsule collider is defined by a line segment, between points
	// A and B. Anything within <dist> distance of that line segment
	// defines the volume of the given capsule.

	glm::vec3 a, b;
	float dist;

	// A, B, and dist, after applying releveant Object -> World transformation
	glm::vec3 a_world, b_world;
	float dist_world;

public:

	CapsuleCollider(glm::vec3 a, glm::vec3 b, float dist);
	~CapsuleCollider();
	void destroy() override;
	void fixedUpdate() override;
	void debugDraw() override;

	bool insideOrIntersects(const glm::vec3& point) const override;
	bool intersects(const BoxCollider& other) const override;
	bool intersects(const CapsuleCollider& other) const override;
	bool intersects(const SphereCollider& other) const override;

	RayHitInfo raycast(const Ray& ray) const override;

	BoxCollider getAABB() const override;
	ColliderType getColliderType() override {
		return ColliderType::CAPSULE;
	};
};