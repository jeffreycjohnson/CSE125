#include "CapsuleCollider.h"
#include "BoxCollider.h"
#include "Renderer.h"

// ctor defined in header

CapsuleCollider::~CapsuleCollider() {

}

void CapsuleCollider::debugDraw()
{
	glm::vec4 color(1,1,0,1); // Yellow for capsule colliders
	if (colliding) {
		color = glm::vec4(1, 0, 0, 1); // Red
	}
	Renderer::drawCapsule(a, b, dist, color);
}

// TODO: Implement CapsuleCollider member functions