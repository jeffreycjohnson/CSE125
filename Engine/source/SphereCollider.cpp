#include "SphereCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Renderer.h"
#include "RenderPass.h"

SphereCollider::SphereCollider(glm::vec3 c, float r) {
	center = c;
	radius = r;
	colliding = previouslyColliding = false;
	collidingStatic = previouslyCollidingStatic = false;
	if (gameObject != nullptr) {
		glm::mat4 matrix = gameObject->transform.getTransformMatrix();
		centerWorld = glm::vec3(matrix * glm::vec4(c, 1));
		radiusWorld = r * gameObject->transform.getWorldScale();
	}
	else {
		// Assume world space coordinates
		LOG("Warning: collider specified with no gameObject! Assuming world space coordinates.");
		centerWorld = c;
		radiusWorld = r;
	}
}

SphereCollider::~SphereCollider() {

}

void SphereCollider::destroy()
{
	Collider::destroy();
}

void SphereCollider::fixedUpdate()
{
	// Make sure our world coordinates are properly updated
	if (gameObject != nullptr) {
		glm::mat4 matrix = gameObject->transform.getTransformMatrix();
		centerWorld = glm::vec3(matrix * glm::vec4(center, 1));
		radiusWorld = radius * gameObject->transform.getWorldScale();
	}
}

void SphereCollider::debugDraw()
{
	if (DebugPass::drawColliders) {
		glm::vec4 color;
		if (colliding || collidingStatic) {
			color = glm::vec4(DebugPass::collidingColor, 1);
		}
		else {
			color = glm::vec4(DebugPass::colliderColor, 1);
		}
		Renderer::drawSphere(centerWorld, radiusWorld, color);
	}
}

bool SphereCollider::insideOrIntersects(const glm::vec3 & point) const
{
	// Assumes point is provided in worldSpace
	glm::vec3 dist = point - centerWorld;
	return (dist.length() <= radiusWorld);
}

bool SphereCollider::intersects(const BoxCollider & other) const
{
	// Okay, YES, this is cheating, but everything will be implemented in one place.
	SphereCollider s = *this;
	return other.intersects(s);
}

bool SphereCollider::intersects(const CapsuleCollider & other) const
{
	SphereCollider s = *this;
	return other.intersects(s);
}

bool SphereCollider::intersects(const SphereCollider & other) const
{
	// Algo modified from Dirk Gregorius' GDC 2013 slides
	float distance = (other.centerWorld - centerWorld).length() - (radiusWorld + other.radiusWorld);
	return distance <= 0;
}

RayHitInfo SphereCollider::intersects(const Ray & ray) const
{
	// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
	// This function returns the earliest hit along the ray of the sphere, provided the hit happens in front of the ray (t >= 0)
	RayHitInfo hit;
	float r_squared = radiusWorld * radiusWorld;
	hit.collider = (Collider*)this;

	// Analytic solution which uses the quadratic formula
	float a = glm::dot(ray.direction, ray.direction);
	float b = 2 * glm::dot(ray.direction, ray.origin);
	float c = glm::dot(ray.origin, ray.origin) - r_squared;

	// t = ( -b +/- sqrt(b^2 - 4ac) ) / 2a

	float discriminant = b * b - 4 * a * c;

	if (discriminant > 0) {
		// Two solutions to quadratic formula. But
		float t0 = (-b - discriminant) / (2 * a);
		float t1 = (-b + discriminant) / (2 * a);
		hit.hitTime = std::min(t0, t1);
		hit.intersects = hit.hitTime > 0;
	}
	else if (discriminant == 0) {
		// Ray intersects 1 point on sphere (e.g. a tangent line)
		hit.hitTime = -b / (2 * a);
		hit.intersects = true;
	}
	else {
		// No solution  -> sqrt( -x ) is imaginary
		hit.intersects = false;
	}

	return hit;
}

BoxCollider SphereCollider::getAABB() const
{
	// Remember to pass in the world coordinates
	return BoxCollider(centerWorld, glm::vec3(radiusWorld,radiusWorld,radiusWorld));
};

glm::vec3 SphereCollider::getCenterWorld() const {
	return centerWorld;
};

float SphereCollider::getRadiusWorld() const {
	return radiusWorld;
};