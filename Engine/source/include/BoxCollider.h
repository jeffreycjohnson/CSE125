#ifndef INCLUDE_BOX_COLLIDER_H
#define INCLUDE_BOX_COLLIDER_H

/*
  Despite it's generic name, we will be using this class only as
  an AXIS-ALIGNED bounding box. For oriented bounding boxes, if
  we ever get to them, we will call them something like OBB or
  OrientedBoxCollider.

  e.g. it will always return ColliderType::AABB
 */

#include "Plane.h"
#include "Collider.h"
#include "Collision.h"
#include <vector>

class BoxCollider : public Collider
{
private:
	// Object space points
	glm::vec3 points[8];

	// points[] - 0 1 2 3 4 5 6 7
	// English  - A B C D E F G H

	// World space points
	glm::vec3 transformPoints[8];

	// *min, *max stored in world space
	float xmin, xmax, ymin, ymax, zmin, zmax;
	bool isAxisAligned;

	// These are provided in local space
    glm::vec3 offset, dimensions;

	// Recomputed in update for global space
	glm::vec3 offsetWorld, dimensionsWorld;

	// Six box planes in WORLD SPACE
	Plane ABCD, ACEG, ABEF, EFGH, BDFH, CDGH;

public:

	static bool drawBoxPoints;

	BoxCollider(glm::vec3 offset, glm::vec3 dimensions);
	~BoxCollider();

	void calculatePlanes();

	void destroy() override;
	void fixedUpdate() override;
	void debugDraw() override;
	void setMinAndMax(const glm::vec3& min, const glm::vec3& max);

	float getWidth();

	bool insideOrIntersects(const glm::vec3& point) const override;

	bool intersects(const BoxCollider& other) const;
	bool intersects(const CapsuleCollider& other) const;
	bool intersects(const SphereCollider& other) const;

	RayHitInfo raycast(const Ray& ray) const override;
	
	BoxCollider getAABB() const override;
	ColliderType getColliderType() override {
		return ColliderType::BOX;
	};
};

#endif