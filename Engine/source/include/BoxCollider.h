#ifndef INCLUDE_BOX_COLLIDER_H
#define INCLUDE_BOX_COLLIDER_H

#include "Component.h"
#include "Collision.h"
#include <vector>

class BoxCollider : public Component
{
private:
	static std::vector<BoxCollider*> colliders; // Naive algorithm
	// A BoxCollider can only be a part of one octreeNode at a time
	NodeId myOctreeNode;
	//std::vector<NodeId> octreeNodes; // For Octree impl, we need to keep track of which nodes we're in

	glm::vec3 points[8];
	glm::vec3 transformPoints[8];
	float xmin, xmax, ymin, ymax, zmin, zmax;
	bool colliding;
    glm::vec3 offset, dimensions;

	bool intersects(BoxCollider& other);

public:
	bool passive;

	BoxCollider(glm::vec3 offset, glm::vec3 dimensions);
	~BoxCollider();
	void destroy() override;
	void update(float) override;
	void debugDraw() override;
	void onCollisionEnter(GameObject* other);

	static void updateColliders();
	static bool checkCollision(int a, int b);
};

#endif