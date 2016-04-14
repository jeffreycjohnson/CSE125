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
	static std::vector<BoxCollider*> colliders; // TODO: Remove naive algorithm once everything is good

	glm::vec3 points[8];
	glm::vec3 transformPoints[8];
	float xmin, xmax, ymin, ymax, zmin, zmax;
	bool isAxisAligned;
    glm::vec3 offset, dimensions;

public:

	BoxCollider(glm::vec3 offset, glm::vec3 dimensions);
	~BoxCollider();
	void destroy() override;
	void update(float) override;
	void debugDraw() override;
	void onCollisionEnter(GameObject* other) override;

	bool insideOrIntersects(const glm::vec3& point) override;
	bool intersects(const BoxCollider& other) override;
	
	BoxCollider getAABB() override;
	ColliderType getColliderType() override {
		return ColliderType::AABB;
	};

	// Part of Michael's original naive algo impl.
	static void updateColliders();
	static bool checkCollision(int a, int b);
};

#endif