#ifndef INCLUDE_BOX_COLLIDER_H
#define INCLUDE_BOX_COLLIDER_H

/*
  Despite it's generic name, we will be using this class only as
  an AXIS-ALIGNED bounding box. For oriented bounding boxes, if
  we ever get to them, we will call them something like OBB or
  OrientedBoxCollider.

  e.g. it will always return ColliderType::AABB
 */

#include "Collider.h"
#include "Collision.h"
#include <vector>

class BoxCollider : public Collider
{
private:
	// Object space points
	glm::vec3 points[8];

	// World space points
	glm::vec3 transformPoints[8];

	// *min, *max stored in world space
	float xmin, xmax, ymin, ymax, zmin, zmax;
	bool isAxisAligned;

	// These are provided in local space
    glm::vec3 offset, dimensions;

	// Recomputed in update for global space
	glm::vec3 offsetWorld, dimensionsWorld;

public:

	BoxCollider(glm::vec3 offset, glm::vec3 dimensions);
	~BoxCollider();
	void destroy() override;
	void fixedUpdate() override;
	void debugDraw() override;
	void onCollisionEnter(GameObject* other) override;
	void setMinAndMax(const glm::vec3& min, const glm::vec3& max);

	bool insideOrIntersects(const glm::vec3& point) const override;

	bool intersects(const BoxCollider& other) const;
	bool intersects(const CapsuleCollider& other) const;
	bool intersects(const SphereCollider& other) const;
	
	BoxCollider getAABB() const override;
	ColliderType getColliderType() override {
		return ColliderType::AABB;
	};
};

#endif