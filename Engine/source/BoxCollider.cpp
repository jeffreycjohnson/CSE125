#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include "GameObject.h"
#include "Input.h"
#include "Renderer.h"
#include "RenderPass.h"
#include <iostream>

bool BoxCollider::drawBoxPoints = false;

BoxCollider::BoxCollider(glm::vec3 offset, glm::vec3 dimensions) : offset(offset), dimensions(dimensions)
{
	float halfW = dimensions.x / 2;
	float halfH = dimensions.y / 2;
	float halfD = dimensions.z / 2;

	// points[] array is in object space!
	points[0] = offset + glm::vec3(halfW, halfH, halfD);    // A
	points[1] = offset + glm::vec3(halfW, halfH, -halfD);   // B
	points[2] = offset + glm::vec3(halfW, -halfH, halfD);   // C
	points[3] = offset + glm::vec3(halfW, -halfH, -halfD);  // D
	points[4] = offset + glm::vec3(-halfW, halfH, halfD);   // E
	points[5] = offset + glm::vec3(-halfW, halfH, -halfD);  // F
	points[6] = offset + glm::vec3(-halfW, -halfH, halfD);  // G
	points[7] = offset + glm::vec3(-halfW, -halfH, -halfD); // H

	// Planes:
	// ABCD (right)    EFGH (left)
	// ACEG (front)    BDFH (back)
	// ABEF (top)      CDGH (bottom)

	colliding = previouslyColliding = false;
	collidingStatic = previouslyCollidingStatic = false;
	passive = true;
	isAxisAligned = true; // For now, ALL box colliders are axis-aligned

	// Force computation of xmin/xmax etc.
	fixedUpdate();
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::calculatePlanes()
{
	// Do mathz here in world space
	glm::vec3 A, B, C, D, E, G;
	A = transformPoints[0];
	B = transformPoints[1];
	C = transformPoints[2];
	D = transformPoints[3];
	E = transformPoints[4];
	G = transformPoints[6];

	ABCD = Plane(A, glm::cross(D - C, A - C));
	ACEG = Plane(A, glm::cross(C - G, E - G));
	ABEF = Plane(A, glm::cross(B - A, E - A));

	EFGH = Plane(E, -ABCD.getNormal());
	BDFH = Plane(B, -ACEG.getNormal());
	CDGH = Plane(C, -ABEF.getNormal());
}

void BoxCollider::fixedUpdate()
{
	if (gameObject != nullptr) {
		glm::mat4 matrix = gameObject->transform.getTransformMatrix();
		offsetWorld = glm::vec3(matrix * glm::vec4(offset, 1));
		dimensionsWorld = glm::vec3(matrix * glm::vec4(dimensions, 1));
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

	// Recalculate the planes if we are an OBB
	//if (!isAxisAligned) {
		calculatePlanes();
	//}
}

BoxCollider BoxCollider::getAABB() const {
	return *this; // TODO: change for OBB later
}

void BoxCollider::debugDraw()
{
	if (DebugPass::drawColliders) {
		glm::vec4 color;
		if (colliding || collidingStatic) {
			color = glm::vec4(DebugPass::collidingColor, 1.0);
		}
		else {
			color = glm::vec4(DebugPass::colliderColor, 1);
		}

		if (rayHitDebugdraw) {
			color = glm::vec4(0.8, 0.8, 0.2, 1);
			rayHitDebugdraw = false; // Reset every draw call
		}

		glm::vec3 dims = dimensions;

		// Spheres around each point
		float sphereRadii = 0.05 * std::max(std::abs(xmax - xmin), std::max(std::abs(ymax - ymin), std::abs(zmin - zmax)));
		
		if (isAxisAligned) {
			// Draw axis-aligned bounding box
			dims = glm::vec3(std::abs(xmax - xmin), std::abs(ymax - ymin), std::abs(zmax - zmin));
			dims /= 2;
			Renderer::drawBox(offsetWorld, dims, color);
			if (drawBoxPoints) {
				Renderer::drawSphere(glm::vec3(xmin, ymin, zmin), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmin, ymax, zmin), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmin, ymin, zmax), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmin, ymax, zmax), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmax, ymin, zmin), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmax, ymax, zmin), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmax, ymin, zmax), sphereRadii, color);
				Renderer::drawSphere(glm::vec3(xmax, ymax, zmax), sphereRadii, color);
			}
		}
		else {
			// Draws oriented bounding box
			dims /= 2;
			Renderer::drawBox(offset, dims, color, &gameObject->transform);
			if (drawBoxPoints) {
				for (int i = 0; i < 8; ++i) {
					Renderer::drawSphere(transformPoints[i], sphereRadii, color); // Global
				}
			}
		}
		// Draw planes (normals)
		/*ABCD.debugDraw(offsetWorld);
		ACEG.debugDraw(offsetWorld);
		ABEF.debugDraw(offsetWorld);
		EFGH.debugDraw(offsetWorld);
		BDFH.debugDraw(offsetWorld);
		CDGH.debugDraw(offsetWorld);*/ // not worth, draw arrow is fucked
	}
}

void BoxCollider::setMinAndMax(const glm::vec3 & min, const glm::vec3 & max)
{
	xmin = min.x;
	ymin = min.y;
	zmin = min.z;
	xmax = max.x;
	ymax = max.y;
	zmax = max.z;

	float halfW = std::abs(xmax - xmin) / 2;
	float halfH = std::abs(ymax - ymin) / 2;
	float halfD = std::abs(zmax - zmin) / 2;

	glm::vec3 offset = (min + max);
	offset /= 2;

	// If it's axis-aligned better set up those world coords
	if (isAxisAligned) {
		offsetWorld = offset;
		dimensionsWorld = glm::vec3(halfW, halfH, halfD);
	}

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
	bool intersectsAABB = (
		this->xmin <= other.xmax && other.xmin <= this->xmax &&
		this->ymin <= other.ymax && other.ymin <= this->ymax &&
		this->zmin <= other.zmax && other.zmin <= this->zmax
	);
	if (!isAxisAligned && intersectsAABB) {
		// If we collide with the OBB's AABB, we will also collide with the OBB,
		// because geometry.

	}
	else {
		return intersectsAABB;
	}
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

}

RayHitInfo BoxCollider::raycast(const Ray & ray) const
{
	// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
	// Because this is a raycast, we only want to register hits with things in front of the ray (t > 0)
	RayHitInfo hit;

	float tmin = (xmin - ray.origin.x) / ray.direction.x;
	float tmax = (xmax - ray.origin.x) / ray.direction.x;

	glm::vec3 tminNorm = glm::vec3(-1.0f, 0.0f, 0.0f);
	glm::vec3 tmaxNorm = glm::vec3(1.0f, 0.0f, 0.0f);

	if (tmin > tmax) {
		std::swap(tmin, tmax);
		std::swap(tminNorm, tmaxNorm);
	}

	float tymin = (ymin - ray.origin.y) / ray.direction.y;
	float tymax = (ymax - ray.origin.y) / ray.direction.y;

	glm::vec3 tyminNorm = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 tymaxNorm = glm::vec3(0.0f, 1.0f, 0.0f);

	if (tymin > tymax) {
		std::swap(tymin, tymax);
		std::swap(tyminNorm, tymaxNorm);
	}

	if ((tmin > tymax) || (tymin > tmax)) {
		hit.intersects = false;
		return hit;
	}

	if (tymin > tmin) {
		tmin = tymin;
		tminNorm = tyminNorm;
	}

	if (tymax < tmax) {
		tmax = tymax;
		tmaxNorm = tymaxNorm;
	}

	float tzmin = (zmin - ray.origin.z) / ray.direction.z;
	float tzmax = (zmax - ray.origin.z) / ray.direction.z;

	glm::vec3 tzminNorm = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 tzmaxNorm = glm::vec3(0.0f, 0.0f, 1.0f);

	if (tzmin > tzmax) {
		std::swap(tzmin, tzmax);
		std::swap(tzminNorm, tzmaxNorm);
	}

	if ((tmin > tzmax) || (tzmin > tmax)) {
		hit.intersects = false;
		return hit;
	}

	if (tzmin > tmin) {
		tmin = tzmin;
		tminNorm = tzminNorm;
	}

	if (tzmax < tmax) {
		tmax = tzmax;
		tmaxNorm = tzmaxNorm;
	}

	float finalT = tmin;
	glm::vec3 finalNorm = tminNorm;
	if (tmin > tmax) {
		finalT = tmax;
		finalNorm = tmaxNorm;
	}

	// Return the tmin/tmax that is closest to the ray's origin
	hit.hitTime = finalT;
	hit.collider = (Collider*)this;
	hit.intersects = true;
	hit.normal = finalNorm;
	hit.point = ray.getPos(finalT);
	return hit;

};

float BoxCollider::getWidth() {
	return xmax - xmin;
}