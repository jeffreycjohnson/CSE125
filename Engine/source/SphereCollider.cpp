#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Renderer.h"
#include "RenderPass.h"

SphereCollider::~SphereCollider() {

}

void SphereCollider::destroy()
{
}

void SphereCollider::update(float)
{
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
		Renderer::drawSphere(center, radius, color);
	}
}

void SphereCollider::onCollisionEnter(GameObject * other)
{
	if (colliding)
		gameObject->collisionEnter(other);
}

bool SphereCollider::insideOrIntersects(const glm::vec3 & point) const
{
	glm::vec3 dist = point - center;
	return (dist.length() <= radius);
}

bool SphereCollider::intersects(const BoxCollider & other) const
{
	// Okay, YES, this is cheating, but everything will be implemented in one place.
	SphereCollider s = *this;
	return other.intersects(s);
}

bool SphereCollider::intersects(const CapsuleCollider & other) const
{
	// TODO: Implement Sphere->Capsule intersection
	return false;
}

bool SphereCollider::intersects(const SphereCollider & other) const
{
	// TODO: Implement Sphere->Sphere intersection
	return false;
}

BoxCollider SphereCollider::getAABB() const
{
	return BoxCollider(center, glm::vec3(radius,radius,radius));
};