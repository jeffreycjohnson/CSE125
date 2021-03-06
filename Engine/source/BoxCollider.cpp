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
	glm::vec3 A, B, C, D, E, F, G;
	A = transformPoints[0];
	B = transformPoints[1];
	C = transformPoints[2];
	D = transformPoints[3];
	E = transformPoints[4];
	F = transformPoints[5];
	G = transformPoints[6];

	ABCD = Plane(A, glm::cross(D - C, A - C));
	ACEG = Plane(A, glm::cross(C - G, E - G));
	ABEF = Plane(A, glm::cross(B - A, E - A));

	EFGH = Plane(A, -ABCD.getNormal());
	BDFH = Plane(A, -ACEG.getNormal());
	CDGH = Plane(A, -ABEF.getNormal());
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
		offsetWorld = offset;
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

	calculatePlanes();

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

	fixedUpdate(); // Recalculate points, etc.
}

void BoxCollider::setAxisAligned(bool axisAligned) {
	isAxisAligned = axisAligned;
};

void BoxCollider::destroy()
{
	Collider::destroy();
}

bool BoxCollider::separatingAxisExists(const BoxCollider& other) const {
	// The SAT states that:
	// "If two convex objects are not penetrating, there exists an axis for which the projection of the objects will not overlap."

	// References:
	// Dirk Gregarious' GDC talk (2013)   (because it's interesting)
	// http://gamedev.stackexchange.com/questions/44500/how-many-and-which-axes-to-use-for-3d-obb-collision-with-sat (OBBs, in particular)
	// http://www.dyn4j.org/2010/01/sat/  (conceptual, SAT in 2D)

	static const int NUM_AXES = 15;
	std::vector<Ray> axes;

	glm::vec3 A, B, C, E, A_other, B_other, C_other, E_other;
	A = transformPoints[0];
	B = transformPoints[1];
	C = transformPoints[2];
	E = transformPoints[4];
	A_other = other.transformPoints[0];
	B_other = other.transformPoints[1];
	C_other = other.transformPoints[2];
	E_other = other.transformPoints[4];

	// 3 axes from face normals of the object
	axes.push_back(Ray(A, ABCD.getNormal())); // right
	axes.push_back(Ray(B, ACEG.getNormal())); // front
	axes.push_back(Ray(C, ABEF.getNormal())); // top

	// 3 axes from face normals of the OTHER object
	axes.push_back(Ray(A_other, other.ABCD.getNormal())); // right
	axes.push_back(Ray(B_other, other.ACEG.getNormal())); // front
	axes.push_back(Ray(C_other, other.ABEF.getNormal())); // top

	// 9 remaining axes from cross products of edges
	axes.push_back(Ray(A, glm::cross(A - B, A_other - B_other)));
	axes.push_back(Ray(A, glm::cross(A - B, A_other - C_other)));
	axes.push_back(Ray(A, glm::cross(A - B, A_other - E_other)));

	axes.push_back(Ray(C, glm::cross(A - C, A_other - B_other)));
	axes.push_back(Ray(C, glm::cross(A - C, A_other - C_other)));
	axes.push_back(Ray(C, glm::cross(A - C, A_other - E_other)));

	axes.push_back(Ray(E, glm::cross(A - E, A_other - B_other)));
	axes.push_back(Ray(E, glm::cross(A - E, A_other - C_other)));
	axes.push_back(Ray(E, glm::cross(A - E, A_other - E_other)));

	// Note that the cross product of two identical vectors = the 0 vector

	/* pseudocode:
	
	for all axes:
		Projection p = this->project()
		Projection c = other.project()

		if ( !overlaps(p,c) )
			return true; // sep axis found
	
	*/

	//assert(axes.size() == NUM_AXES);
	glm::vec3 zero(0);

	for (int axis = 0; axis < NUM_AXES; ++axis) {
		float my_proj_min = INFINITY, my_proj_max = -INFINITY;
		float ot_proj_min = INFINITY, ot_proj_max = -INFINITY;

		if (axes[axis].direction == zero || glm::isnan(axes[axis].direction).b ) {
			continue;
		}

		for (int vertex = 0; vertex < 8; ++vertex) {
			// Project each vertex of our box onto the axis (our std::vector of Rays)
			// The dot product gives us the signed distance to that point.
			float temp = glm::dot(transformPoints[vertex], axes[axis].direction);

			// Debug assertion to make sure we don't dot against the 0 vector
			ASSERT(!std::isnan(temp), "Separating Axis Theorem: projection onto OBB axis is NaN");

			// Build the interval (e.g. the object's "shadow") along the axis over time
			my_proj_min = std::min(my_proj_min, temp);
			my_proj_max = std::max(my_proj_max, temp);
		}

		// Now, project the other box's vertices onto the same axis
		for (int otherVertex = 0; otherVertex < 8; ++otherVertex) {
			float temp = glm::dot(other.transformPoints[otherVertex], axes[axis].direction);
			ASSERT(!std::isnan(temp), "Separating Axis Theorem: projection onto OBB axis is NaN");
			ot_proj_min = std::min(ot_proj_min, temp);
			ot_proj_max = std::max(ot_proj_max, temp);
		}

		// Compare the intervals
		// [my_proj_min, my_proj_max], [ot_proj_min, ot_proj_max]
		if (my_proj_max < ot_proj_min || ot_proj_max < my_proj_min) {
			return true; // We found a separating axis between the two boxes!
		}
	}

	return false; // No separating axis was detected

}

void BoxCollider::rayOBB(const Ray & ray, RayHitInfo& hit) const
{
	// Real Time Rendering method (2nd edition)

	float tMin = -INFINITY;
	float tMax = INFINITY;

	glm::vec3 p = offsetWorld - ray.origin;

	// Compute axes
	glm::vec3 A, B, C, E, F;
	A = transformPoints[0]; B = transformPoints[1];
	C = transformPoints[2]; E = transformPoints[4];
	F = transformPoints[5];

	// x
	glm::vec3 a_u = ABCD.getNormal();
	glm::vec3 a_u_max = -a_u;
	float h_u = glm::distance(A, E) / 2.0f;
	
	// y
	glm::vec3 a_v = ABEF.getNormal();
	glm::vec3 a_v_max = -a_v;
	float h_v = glm::distance(A, C) / 2.0f;

	// z
	glm::vec3 a_w = ACEG.getNormal();
	glm::vec3 a_w_max = -a_w;
	float h_w = glm::distance(A, B) / 2.0f;

	glm::vec3 min_norm, max_norm;

	// Axis U (x)
	{
		float e = glm::dot(a_u, p);
		float f = glm::dot(a_u, ray.direction);
		if (std::abs(f) > FLT_EPSILON) {
			float t1 = (e + h_u) / f;
			float t2 = (e - h_u) / f;

			if (t1 > t2) {
				std::swap(t1, t2);
				std::swap(a_u, a_u_max);
			}

			if (t1 > tMin) {
				tMin = t1;
				min_norm = a_u;
			}

			if (t2 < tMax) {
				tMax = t2;
				max_norm = a_u_max;
			}

			if (tMin > tMax || tMax < 0) {
				// No intersection
				hit.intersects = false;
				hit.hitTime = 0; return;
			}
		}
		else if ((-e - h_u) > 0 || (-e + h_u) < 0) {
			// No intersection
			hit.intersects = false;
			hit.hitTime = 0; return;
		}
	}

	// Axis V (y)
	{
		float e = glm::dot(a_v, p);
		float f = glm::dot(a_v, ray.direction);
		if (std::abs(f) > FLT_EPSILON) {
			float t1 = (e + h_v) / f;
			float t2 = (e - h_v) / f;

			if (t1 > t2) {
				std::swap(t1, t2);
				std::swap(a_v, a_v_max);
			}

			if (t1 > tMin) {
				tMin = t1;
				min_norm = a_v;
			}

			if (t2 < tMax) {
				tMax = t2;
				max_norm = a_v_max;
			}

			if (tMin > tMax || tMax < 0) {
				// No intersection
				hit.intersects = false;
				hit.hitTime = 0; return;
			}
		}
		else if ((-e - h_v) > 0 || (-e + h_v) < 0) {
			// No intersection
			hit.intersects = false;
			hit.hitTime = 0; return;
		}
	}

	// Axis W (z)
	{
		float e = glm::dot(a_w, p);
		float f = glm::dot(a_w, ray.direction);
		if (std::abs(f) > FLT_EPSILON) {
			float t1 = (e + h_w) / f;
			float t2 = (e - h_w) / f;

			if (t1 > t2) {
				std::swap(t1, t2);
				std::swap(a_w, a_w_max);
			}

			if (t1 > tMin) {
				tMin = t1;
				min_norm = a_w;
			}

			if (t2 < tMax) {
				tMax = t2;
				max_norm = a_w_max;
			}

			if (tMin > tMax || tMax < 0) {
				// No intersection
				hit.intersects = false;
				hit.hitTime = 0; return;
			}
		}
		else if ((-e - h_w) > 0 || (-e + h_w) < 0) {
			// No intersection
			hit.intersects = false;
			hit.hitTime = 0; return;
		}
	}

	if (tMin > 0) {
		hit.hitTime = tMin;
		hit.normal = min_norm;
		hit.intersects = true;
		return;
	}
	else {
		hit.hitTime = tMax;
		hit.normal = max_norm;
		hit.intersects = true;
		return;
	}
};

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
		if (intersectsAABB) {
			// If the AABBs overlap, we will need to use the Separating Axis Theorem.
			// If no separating axis exists between this box and the other, then
			// the objects must penetrate each other.
			return !separatingAxisExists(other);
		}
		else {
			return false;
		}
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

	// Arvo's original method
	if (c.x < xmin) {
		e = c.x - xmin;
		d = d + e * e;
	}
	else if (c.x > xmax) {
		e = c.x - xmax;
		d += e * e;
	}

	if (c.y < ymin) {
		e = c.y - ymin;
		d = d + e * e;
	}
	else if (c.y > ymax) {
		e = c.y - ymax;
		d = d + e * e;
	}

	if (c.z < zmin) {
		e = c.z - zmin;
		d += e * e;
	}
	else if (c.z > zmax) {
		e = c.z - zmax;
		d = d + e * e;
	}

	return (d <= r_squared);

}

void BoxCollider::rayAABB(const Ray & ray, RayHitInfo& hit) const
{
	// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
	// Because this is a raycast, we only want to register hits with things in front of the ray (t > 0)

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
		return;
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
		return;
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
	hit.intersects = true;
	hit.normal = finalNorm;
}

RayHitInfo BoxCollider::raycast(const Ray & ray) const
{
	RayHitInfo hit;
	hit.collider = (Collider*)this;
	if (isAxisAligned) {
		rayAABB(ray, hit);
	}
	else {
		rayOBB(ray, hit);
		return hit;
	}
	if (hit.intersects) {
		hit.point = ray.getPos(hit.hitTime);
	}
	return hit;
};

float BoxCollider::getWidth() {
	// Distance between A & E
	return glm::distance(transformPoints[0], transformPoints[4]);
}

float BoxCollider::getHeight() {
	// Distance between A & C
	return glm::distance(transformPoints[0], transformPoints[2]);
}

float BoxCollider::getDepth() {
	// Distance between A & B
	return glm::distance(transformPoints[0], transformPoints[1]);
}

float BoxCollider::getYMax()
{
	return ymax;
}

float BoxCollider::getYMin()
{
	return ymin;
}
