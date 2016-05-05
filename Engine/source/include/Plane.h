/*
  Defines a plane, and useful collision routines against that plane.
  Will be very valuable when implementing oriented bounding boxes (perhaps)
  and extremely useful when implementing frustum culling.

  We will define a point being "inside" the plane, if it is in front of
  the normal. The normal of the plane points inward from the face
  of the plane.

            |
            |  (Normal)
  (outside) |   ----->   (inside)
            |
            |

  MATHS:
  http://www.lighthouse3d.com/tutorials/maths/plane/
  http://mathworld.wolfram.com/Plane.html
*/
#include "ForwardDecs.h"
#include "Collision.h"

class Plane {
private:
	// Ax + By + Cz + D = 0
	float A, B, C, D; // May be useful
	glm::vec3 N;      // Plane normal

	static const int INSIDE  = 0;
	static const int OUTSIDE = 1;

public:
	Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);
	Plane(glm::vec3 p0, glm::vec3 normal);
	~Plane();

	bool pointInside(const glm::vec3&);

	// Returns the SIGNED distance from the point to the plane!
	// If you need absolute distance, remeber to use std::abs
	float distanceToPoint(const glm::vec3&);

	RayHitInfo intersects(const Ray& ray);
};