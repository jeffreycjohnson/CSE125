#ifndef INCLUDE_BOX_COLLIDER_H
#define INCLUDE_BOX_COLLIDER_H

#include "Collider.h"
#include "Collision.h"
#include <vector>

class BoxCollider : public Collider
{
private:
	static std::vector<BoxCollider*> colliders; // Naive algorithm

	glm::vec3 points[8];
	glm::vec3 transformPoints[8];
	float xmin, xmax, ymin, ymax, zmin, zmax;
	bool colliding;
	bool isAxisAligned;
    glm::vec3 offset, dimensions;

public:
	bool passive;

	BoxCollider(glm::vec3 offset, glm::vec3 dimensions);
	~BoxCollider();
	void destroy() override;
	void update(float) override;
	void debugDraw() override;
	void onCollisionEnter(GameObject* other);

	bool intersects(const BoxCollider& other);
	BoxCollider getAABB();

	static void updateColliders();
	static bool checkCollision(int a, int b);
};

#endif