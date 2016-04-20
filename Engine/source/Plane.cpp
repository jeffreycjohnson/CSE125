#include "Plane.h"

Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
	glm::vec3 u = p1 - p0;
	glm::vec3 v = p2 - p0;
	N = glm::normalize(glm::cross(u, v));

	A = N.x;
	B = N.y;
	C = N.z;
	D = glm::dot(-N, p0);
};

Plane::Plane(glm::vec3 p0, glm::vec3 normal) {
	N = glm::normalize(normal);
	D = glm::dot(-N, p0);
};

Plane::~Plane() {

}

// See header for clarification on Plane semantics
bool Plane::pointInside(const glm::vec3& point) {
	float signedDist = distanceToPoint(point);
	return (signedDist > 0);
}


// Math reference: http://mathworld.wolfram.com/Plane.html
// "If the sign is positive then the point is on the side that 
//  agrees with the normal n, otherwise it is on the other side."
float Plane::distanceToPoint(const glm::vec3& point) {
	float distance = (A * point.x + B * point.y + C * point.z + D);
	distance /= std::sqrtf(A * A + B * B + C * C);
	return distance;
}