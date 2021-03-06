/*
  Implements collision routines against a sphere.
 */
#pragma once
#include "ForwardDecs.h"
#include "Collider.h"

class SphereCollider : public Collider
{
private:
	// Local (Object) Space
	glm::vec3 center;
	float radius;

	// World Space
	glm::vec3 centerWorld;
	float radiusWorld;

public:
	
	SphereCollider(glm::vec3 c, float r);
	~SphereCollider();
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
		return ColliderType::SPHERE;
	};

	// Sphere-specific functions
	glm::vec3 getCenterWorld() const;
	float getRadiusWorld() const;

};