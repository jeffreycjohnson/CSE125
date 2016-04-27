#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include "GameObject.h"
#include "Input.h"
#include "Renderer.h"
#include "RenderPass.h"

BoxCollider::BoxCollider(glm::vec3 offset, glm::vec3 dimensions) : offset(offset), dimensions(dimensions)
{
	float halfW = dimensions.x / 2;
	float halfH = dimensions.y / 2;
	float halfD = dimensions.z / 2;

	// points[] array is in object space!
	points[0] = offset + glm::vec3(halfW, halfH, halfD);
	points[1] = offset + glm::vec3(halfW, halfH, -halfD);
	points[2] = offset + glm::vec3(halfW, -halfH, halfD);
	points[3] = offset + glm::vec3(halfW, -halfH, -halfD);
	points[4] = offset + glm::vec3(-halfW, halfH, halfD);
	points[5] = offset + glm::vec3(-halfW, halfH, -halfD);
	points[6] = offset + glm::vec3(-halfW, -halfH, halfD);
	points[7] = offset + glm::vec3(-halfW, -halfH, -halfD);
	//colliders.push_back(this); // TODO: Remove naive implementation once octree is working
	colliding = false;
	previouslyColliding = false;
	passive = true;
	isAxisAligned = true; // For now, ALL box colliders are axis-aligned

	// Force computation of xmin/xmax etc.
	update(0.0f);
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::update(float)
{
	if (gameObject != nullptr) {
		glm::mat4 matrix = gameObject->transform.getTransformMatrix();
		for (int i = 0; i < 8; i++)
		{
			transformPoints[i] = glm::vec3(matrix * glm::vec4(points[i], 1));
		}
	}
	else {
		// If no gameObject (and therefore, no transform) is specified, assume world coords already
		for (int i = 0; i < 8; ++i) {
			transformPoints[i] = points[i];
		}
	}

	// Calculate axis aligned bounding box
	xmin = xmax = transformPoints[0].x;
	ymin = ymax = transformPoints[0].y;
	zmin = zmax = transformPoints[0].z;
	for (int i = 1; i < 8; i++)
	{
		if (xmin > transformPoints[i].x)
			xmin = transformPoints[i].x;
		if (xmax < transformPoints[i].x)
			xmax = transformPoints[i].x;

		if (ymin > transformPoints[i].y)
			ymin = transformPoints[i].y;
		if (ymax < transformPoints[i].y)
			ymax = transformPoints[i].y;

		if (zmin > transformPoints[i].z)
			zmin = transformPoints[i].z;
		if (zmax < transformPoints[i].z)
			zmax = transformPoints[i].z;
	}
}

BoxCollider BoxCollider::getAABB() const {
	return *this;
}

void BoxCollider::debugDraw()
{
	if (DebugPass::drawColliders) {
		glm::vec4 color;
		if (colliding) {
			color = glm::vec4(DebugPass::collidingColor, 1.0);
		}
		else {
			color = glm::vec4(DebugPass::colliderColor, 1);
		}

		// Draw all 8 points of the box
		for (int i = 0; i < 8; ++i) {
			Renderer::drawSphere(transformPoints[i], 0.2f, color); // Global
		}

		glm::vec3 dims = dimensions;
		dims /= 2;
		Renderer::drawBox(offset, dims, color, &gameObject->transform);
	}
}

void BoxCollider::onCollisionEnter(GameObject* other)
{
}

void BoxCollider::setMinAndMax(const glm::vec3 & min, const glm::vec3 & max)
{
	xmin = min.x;
	ymin = min.y;
	zmin = min.z;
	xmax = max.x;
	ymax = max.y;
	zmax = max.z;

	float halfW = std::abs(xmax - xmin);
	float halfH = std::abs(ymax - ymin);
	float halfD = std::abs(zmax - zmin);

	glm::vec3 offset = (min + max);
	offset /= 2;

	// To prevent update() from fucking it up
	points[0] = offset + glm::vec3(halfW, halfH, halfD);
	points[1] = offset + glm::vec3(halfW, halfH, -halfD);
	points[2] = offset + glm::vec3(halfW, -halfH, halfD);
	points[3] = offset + glm::vec3(halfW, -halfH, -halfD);
	points[4] = offset + glm::vec3(-halfW, halfH, halfD);
	points[5] = offset + glm::vec3(-halfW, halfH, -halfD);
	points[6] = offset + glm::vec3(-halfW, -halfH, halfD);
	points[7] = offset + glm::vec3(-halfW, -halfH, -halfD);

	update(0.0f); // Recalculate points, etc.
}

void BoxCollider::destroy()
{
	Collider::destroy();
}

bool BoxCollider::insideOrIntersects(const glm::vec3& point) const {
	return (
		this->xmin <= point.x && point.x <= this->xmax &&
		this->ymin <= point.y && point.y <= this->ymax &&
		this->zmin <= point.z && point.z <= this->zmax
	);
}

bool BoxCollider::intersects(const BoxCollider& other) const {
	return (
		this->xmin <= other.xmax && other.xmin <= this->xmax &&
		this->ymin <= other.ymax && other.ymin <= this->ymax &&
		this->zmin <= other.zmax && other.zmin <= this->zmax
	);
}

bool BoxCollider::intersects(const CapsuleCollider & other) const
{
	// TODO: Implement Box->Capsule intersection
	return false;
}

bool BoxCollider::intersects(const SphereCollider & other) const 
{
	// TODO: this is wrong, or maybe spheres are broken, idk

	// Using algorithm from this paper: http://www.mrtc.mdh.se/projects/3Dgraphics/paperF.pdf
	// From section (3) {branch elimination & vectorization}, and no, I don't understand it

	float d = 0;
	float e;
	glm::vec3 c = other.getCenterWorld();
	float radius = other.getRadiusWorld();
	float r_squared = radius * radius;

	e = std::fmaxf(xmin - c.x, 0) + std::fmaxf(c.x - xmax, 0);
	if (e <= radius) return false;
	d += e * e;

	e = std::fmaxf(ymin - c.y, 0) + std::fmaxf(c.y - ymax, 0);
	if (e <= radius) return false;
	d += e * e;

	e = std::fmaxf(zmin - c.z, 0) + std::fmaxf(c.z - zmax, 0);
	if (e <= radius) return false;
	d += e * e;

	if (d <= r_squared) {
		return true;
	}
	else {
		return false;
	}

};