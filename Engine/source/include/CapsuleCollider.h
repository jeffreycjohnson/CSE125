// TODO: Create definitions for CapsuleCollider, and implement

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

public:

	CapsuleCollider(glm::vec3 a, glm::vec3 b, float dist) : a(a), b(b), dist(dist) {};
	~CapsuleCollider();
	void destroy() override;
	void update(float) override;
	void debugDraw() override;
	void onCollisionEnter(GameObject* other) override;

	bool insideOrIntersects(const glm::vec3& point) const override;
	bool intersects(const BoxCollider& other) const override;
	bool intersects(const CapsuleCollider& other) const override;
	bool intersects(const SphereCollider& other) const override;

	BoxCollider getAABB() const override;
	ColliderType getColliderType() override {
		return ColliderType::CAPSULE;
	};
};