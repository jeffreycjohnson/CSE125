/*
  Implements collision routines against a sphere.
 */

#include "ForwardDecs.h"
#include "Collider.h"

class SphereCollider : public Collider
{
private:
	glm::vec3 center;
	float radius;

public:

	SphereCollider(glm::vec3 c, float r) : center(c), radius(r) {};
	~SphereCollider();
	void destroy() override;
	void update(float) override;
	void debugDraw() override;
	void onCollisionEnter(GameObject* other) override;

	bool insideOrIntersects(const glm::vec3& point) override;
	bool intersects(const BoxCollider& other) override;
	//bool intersects(const CapsuleCollider& other) override;
	//bool intersects(const SphereCollider& other) override;

	BoxCollider getAABB() override;
	ColliderType getColliderType() override {
		return ColliderType::SPHERE;
	};
};