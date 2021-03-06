#include "Plane.h"
#include "Collision.h"
#include "Renderer.h"

Plane::Plane()
{
	N = glm::vec3(0, 0, 0);
	A = B = C = D = 0.0f;
	normalBase = glm::vec3(0);
}

Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
	glm::vec3 u = p1 - p0;
	glm::vec3 v = p2 - p0;
	N = glm::normalize(glm::cross(u, v));
	normalBase = p0;

	A = N.x;
	B = N.y;
	C = N.z;
	D = glm::dot(-N, p0);
};

Plane::Plane(glm::vec3 p0, glm::vec3 normal) {
	N = glm::normalize(normal);
	A = N.x;
	B = N.y;
	C = N.z;
	D = glm::dot(-N, p0);
	normalBase = p0;
};

Plane::~Plane() {

}

// See header for clarification on Plane semantics
bool Plane::pointInside(const glm::vec3& point) const {
	float signedDist = distanceToPoint(point);
	return (signedDist > 0);
}


// Math reference: http://mathworld.wolfram.com/Plane.html
// "If the sign is positive then the point is on the side that 
//  agrees with the normal n, otherwise it is on the other side."
float Plane::distanceToPoint(const glm::vec3& point) const {
	float distance = (A * point.x + B * point.y + C * point.z + D);
	distance /= std::sqrtf(A * A + B * B + C * C);
	return distance;
}

RayHitInfo Plane::intersects(const Ray & ray) const
{
	RayHitInfo hitInfo;

	// Ray:    P         = O + t * D
	// Plane:  N * P + d = 0

	// Substitute:   N * (O + t * D) + d = 0
	//               t = -(N * O + d) / (N * D)

	float denominator = glm::dot(N, ray.direction);
	float numerator = -(glm::dot(N, ray.origin) + D);

	if (denominator > 0.0f - FLT_EPSILON && denominator < 0.0f + FLT_EPSILON ) {
		// If the denominator is within FLT_EPSILON of zero, no intersection
		hitInfo.intersects = false;
		// Remember: If dot product between two vectors is 0, the vectors are orthogonal to each other
	}
	else {
		hitInfo.hitTime = numerator / denominator;
		hitInfo.intersects = true;
		hitInfo.collider = nullptr;
		hitInfo.point = ray.getPos(hitInfo.hitTime);
		hitInfo.normal = N;
	}

	return hitInfo;
}

glm::vec3 Plane::getNormal() const
{
	return N;
}

void Plane::debugDraw(const glm::vec3& pos) const {
	Renderer::drawArrow(pos, N, glm::vec4(1,1,1,1));
}