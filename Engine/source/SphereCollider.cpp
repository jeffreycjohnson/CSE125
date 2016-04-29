#include "SphereCollider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Renderer.h"
#include "RenderPass.h"

SphereCollider::SphereCollider(glm::vec3 c, float r) {
	center = c;
	radius = r;
	colliding = previouslyColliding = false;
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
		if (colliding) {
			color = glm::vec4(DebugPass::collidingColor, 1);
		}
		else {
			color = glm::vec4(DebugPass::colliderColor, 1);
		}
		Renderer::drawSphere(centerWorld, radiusWorld, color);
	}
}

void SphereCollider::onCollisionEnter(GameObject * other)
{
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
	float distance = (other.centerWorld - centerWorld).length();
	return (distance <= radiusWorld + other.radiusWorld);
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