#include "CapsuleCollider.h"
#include "BoxCollider.h"
#include "Renderer.h"
#include "RenderPass.h"

CapsuleCollider::CapsuleCollider(glm::vec3 a, glm::vec3 b, float dist) {
	this->a = a;
	this->b = b;
	this->dist = dist;
	colliding = previouslyColliding = false;
	if (gameObject != nullptr) {
		glm::mat4 matrix = gameObject->transform.getTransformMatrix();
		this->a_world = glm::vec3(matrix * glm::vec4(a,1));
		this->b_world = glm::vec3(matrix * glm::vec4(b,1));
		this->dist_world = dist * gameObject->transform.getWorldScale();
	}
	else {
		// Assume world space coordinates
		LOG("Warning: collider specified with no gameObject! Assuming world space coordinates.");
		this->a_world = a;
		this->b_world = b;
		this->dist_world = dist;
	}
}

CapsuleCollider::~CapsuleCollider() {

}

void CapsuleCollider::destroy() {
	Collider::destroy();
};

void CapsuleCollider::fixedUpdate() {
	// Make sure our positional information is updated
	if (gameObject != nullptr) {
		glm::mat4 matrix = gameObject->transform.getTransformMatrix();
		this->a_world = glm::vec3(matrix * glm::vec4(a, 1));
		this->b_world = glm::vec3(matrix * glm::vec4(b, 1));
		this->dist_world = dist * gameObject->transform.getWorldScale();
	}
};

void CapsuleCollider::debugDraw()
{
	if (DebugPass::drawColliders) {
		glm::vec4 color(DebugPass::colliderColor, 1);
		if (colliding) {
			color = glm::vec4(DebugPass::collidingColor, 1);
		}
		Renderer::drawCapsule(a, b, dist, color);
	}
}

bool CapsuleCollider::insideOrIntersects(const glm::vec3& point) const {
	// TODO: Implement this for capsules
	return false;
}

bool CapsuleCollider::intersects(const BoxCollider& other) const {
	// TODO: Box -> Capsule intersection
	return false;
};

bool CapsuleCollider::intersects(const CapsuleCollider& other) const {
	// TODO: Capsule -> Capsule intersection 
	return false;
};

bool CapsuleCollider::intersects(const SphereCollider& other) const {
	// TODO: Capsule -> Sphere intersection
	return false;
}

RayHitInfo CapsuleCollider::intersects(const Ray & ray) const
{
	return RayHitInfo(); // TODO: CapsuleCollider implement ray
};

BoxCollider CapsuleCollider::getAABB() const {
	glm::vec3 line = a_world - b_world;
	return BoxCollider(line, glm::vec3(dist_world, dist_world, dist_world)); // TODO: This is completely incorrect, I'm just writing it so it compiles
};