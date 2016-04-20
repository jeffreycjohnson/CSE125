#include "CapsuleCollider.h"
#include "BoxCollider.h"
#include "Renderer.h"
#include "RenderPass.h"

// ctor defined in header

CapsuleCollider::~CapsuleCollider() {

}

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

// TODO: Implement CapsuleCollider member functions