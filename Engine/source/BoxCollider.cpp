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
			assert(!std::isnan(temp));

			// Build the interval (e.g. the object's "shadow") along the axis over time
			my_proj_min = std::min(my_proj_min, temp);
			my_proj_max = std::max(my_proj_max, temp);
		}

		// Now, project the other box's vertices onto the same axis
		for (int otherVertex = 0; otherVertex < 8; ++otherVertex) {
			float temp = glm::dot(other.transformPoints[otherVertex], axes[axis].direction);
			assert(!std::isnan(temp));
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

	// http://what-when-how.com/advanced-methods-in-computer-graphics/collision-detection-advanced-methods-in-computer-graphics-part-3/

	glm::vec3 A, B, C, E, F;
	A = transformPoints[0];
	B = transformPoints[1];
	C = transformPoints[2];
	E = transformPoints[4];
	F = transformPoints[5];

	// x
	glm::vec3 e1 = A - E;
	glm::vec3 e1_min_norm, e1_max_norm;
	float w1 = e1.length();

	// y
	glm::vec3 e2 = A - C;
	glm::vec3 e2_min_norm, e2_max_norm;
	float w2 = e2.length();

	// z
	glm::vec3 e3 = A - B;
	glm::vec3 e3_min_norm, e3_max_norm;
	float w3 = e3.length();

	// using semantics from example

	glm::vec3 m = ray.direction;
	glm::vec3 p = ray.origin;
	glm::vec3 c = offsetWorld;

	float t1Min, t1Max, t2Min, t2Max, t3Min, t3Max, tMin, tMax;

	auto dot = glm::dot(m, e1);
	if (dot > 0) {
		e1_min_norm = EFGH.getNormal();
		e1_max_norm = ABCD.getNormal();
		t1Min = (-w1 - glm::dot(p, e1)) / glm::dot(m, e1);
		t1Max = (w1 - glm::dot(p, e1)) / glm::dot(m, e1);
	}
	else if (dot < 0) {
		e1_min_norm = ABCD.getNormal(); 
		e1_max_norm = EFGH.getNormal();
		t1Min = (w1 - glm::dot(p, e1)) / glm::dot(m, e1);
		t1Max = (-w1 - glm::dot(p, e1)) / glm::dot(m, e1);
	}

	dot = glm::dot(m, e2);
	if (dot > 0) {
		e2_min_norm = ABEF.getNormal();
		e2_max_norm = CDGH.getNormal();
		t2Min = (-w1 - glm::dot(p, e2)) / glm::dot(m, e2);
		t2Max = (w1 - glm::dot(p, e2)) / glm::dot(m, e2);
	}
	else if (dot < 0) {
		e2_min_norm = CDGH.getNormal();
		e2_max_norm = ABEF.getNormal();
		t2Min = (w1 - glm::dot(p, e2)) / glm::dot(m, e2);
		t2Max = (-w1 - glm::dot(p, e2)) / glm::dot(m, e2);
	}

	dot = glm::dot(m, e3);
	if (dot > 0) {
		e3_min_norm = BDFH.getNormal();
		e3_max_norm = ACEG.getNormal();
		t3Min = (-w1 - glm::dot(p, e3)) / glm::dot(m, e3);
		t3Max = (w1 - glm::dot(p, e3)) / glm::dot(m, e3);
	}
	else if (dot < 0) {
		e3_min_norm = ACEG.getNormal();
		e3_max_norm = BDFH.getNormal();
		t3Min = (w1 - glm::dot(p, e3)) / glm::dot(m, e3);
		t3Max = (-w1 - glm::dot(p, e3)) / glm::dot(m, e3);
	}

	tMin = std::max(std::max(t1Min, t2Min), std::max(t2Min, t3Min));
	tMax = std::min(std::min(t1Max, t2Max), std::min(t2Max, t3Max));

	// Shitty way to do this, but screw it
	if (tMin == t1Min) {
		hit.normal = e1_min_norm;
	}
	else if (tMin == t2Min) {
		hit.normal = e2_min_norm;
	}
	else {
		hit.normal = e3_min_norm;
	}

	if (tMin < tMax) {
		hit.hitTime = tMin;
		hit.intersects = true;
	}
	else {
		hit.intersects = false;
	}

	// Courtesy of: http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/

	// Intersection method from Real-Time Rendering and Essential Mathematics for Games
/*
	float tMin = Octree::RAY_MIN;
	float tMax = Octree::RAY_MAX;

	// **aaaahhhh*** Since this method transforms the Ray into Object space, we need to use
	// object min & max points

	glm::vec3 localMin(INFINITY), localMax(-INFINITY);
	for (int i = 0; i < 8; ++i) {
		localMin = glm::vec3(std::min(localMin.x, points[i].x), std::min(localMin.y, points[i].y), std::min(localMin.z, points[i].z));
		localMax = glm::vec3(std::max(localMax.x, points[i].x), std::max(localMax.y, points[i].y), std::max(localMax.z, points[i].z));
	}

	glm::vec3 n_min, n_max; // Normals at min & max intersection points

	glm::mat4 ModelMatrix = gameObject->transform.getTransformMatrix();
	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z); //  = offsetWorld; //

	glm::vec3 delta = OBBposition_worldspace - ray.origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z); // ABCD.getNormal();//
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray.direction, xaxis);

		if (fabs(f) > 0.001f) { // Standard case

			float t1 = (e + localMin.x) / f; // Intersection with the "left" plane
			float t2 = (e + localMax.x) / f; // Intersection with the "right" plane
											 // t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			glm::vec3 n_near = EFGH.getNormal(); // left plane
			glm::vec3 n_far = ABCD.getNormal();  // right plane

											 // We want t1 to represent the nearest intersection, 
											 // so if it's not the case, invert t1 and t2
			if (t1>t2) {
				float w = t1; t1 = t2; t2 = w; // swap t1 and t2
				std::swap(n_near, n_far);
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if (t2 < tMax) {
				tMax = t2;
				n_max = n_far;
			}
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if (t1 > tMin) {
				tMin = t1;
				n_min = n_near;
			}

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin) {
				hit.intersects = false;
				return;
			}

		}
		else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if (-e + localMin.x > 0.0f || -e + localMax.x < 0.0f) {
				hit.intersects = false;
				return;
			}
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z); //  = ABEF.getNormal();//
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray.direction, yaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + localMin.y) / f; // bottom plane
			float t2 = (e + localMax.y) / f; // top plane

			glm::vec3 n_near = CDGH.getNormal();  // bottom plane
			glm::vec3 n_far = ABEF.getNormal(); // top plane

			if (t1>t2) { 
				std::swap(t1, t2);
				std::swap(n_near, n_far);
			}

			if (t2 < tMax) {
				tMax = t2;
				n_max = n_far;
			}
			if (t1 > tMin) {
				tMin = t1;
				n_min = n_near;
			}
			if (tMin > tMax) {
				hit.intersects = false;
				return;
			}

		}
		else {
			if (-e + localMin.y > 0.0f || -e + localMax.y < 0.0f) {
				hit.intersects = false;
				return;
			}
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z); //  zaxis = ACEG.getNormal();//
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray.direction, zaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + localMin.z) / f; // front plane
			float t2 = (e + localMax.z) / f; // back plane

			glm::vec3 n_near = BDFH.getNormal();  // back plane
			glm::vec3 n_far = ACEG.getNormal(); // front plane

			if (t1>t2) {
				std::swap(t1, t2);
				std::swap(n_near, n_far);
			}

			if (t2 < tMax) {
				tMax = t2;
				n_max = n_far;
			}
			if (t1 > tMin) {
				tMin = t1;
				n_min = n_near;
			}
			if (tMin > tMax) {
				hit.intersects = false;
				return;
			}
		}
		else {
			if (-e + localMin.z > 0.0f || -e + localMax.z < 0.0f) {
				hit.intersects = false;
				return;
			}
		}
	}


	hit.hitTime = tMin;
	hit.normal = n_min; // pls?
	hit.intersects = true;
	return; */
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
	/*
	e = std::fmaxf(xmin - c.x, 0) + std::fmaxf(c.x - xmax, 0);
	if (e <= radius) return false;
	d += e * e;

	e = std::fmaxf(ymin - c.y, 0) + std::fmaxf(c.y - ymax, 0);
	if (e <= radius) return false;
	d += e * e;

	e = std::fmaxf(zmin - c.z, 0) + std::fmaxf(c.z - zmax, 0);
	if (e <= radius) return false;
	d += e * e;*/

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
		//rayAABB(ray, hit);
		rayOBB(ray, hit);
		return hit;
		// The Lazy Way (TM)
		/*std::vector<RayHitInfo> hits;
		hits.push_back(ABCD.intersects(ray));
		hits.push_back(ABEF.intersects(ray));
		hits.push_back(ACEG.intersects(ray));
		hits.push_back(BDFH.intersects(ray));
		hits.push_back(CDGH.intersects(ray));
		hits.push_back(EFGH.intersects(ray));

		RayHitInfo finalHit;
		finalHit.hitTime = INFINITY;
		for (auto hit : hits) {

			// All plane normals for box are pointing outward
			// For a point to lie in the OBB, it must have signed distance
			// to the plane of <= 0 to ALL planes
			auto point = ray.getPos(hit.hitTime);
			if (ABCD.distanceToPoint(point) > 0) {
				hit.intersects = false; continue;
			}
			if (ABEF.distanceToPoint(point) > 0) {
				hit.intersects = false; continue;
			}
			if (ACEG.distanceToPoint(point) > 0) {
				hit.intersects = false; continue;
			}
			if (BDFH.distanceToPoint(point) > 0) {
				hit.intersects = false; continue;
			}
			if (CDGH.distanceToPoint(point) > 0) {
				hit.intersects = false; continue;
			}
			if (EFGH.distanceToPoint(point) > 0) {
				hit.intersects = false; continue;
			}

			if (hit.intersects && hit.hitTime < finalHit.hitTime) {
				finalHit = hit;
			}

		}*/
		//return finalHit;
	}
	if (hit.intersects) {
		hit.point = ray.getPos(hit.hitTime);
	}
	return hit;
};

float BoxCollider::getWidth() {
	return xmax - xmin;
}

float BoxCollider::getHeight() {
	return ymax - ymin;
}

float BoxCollider::getDepth() {
	return zmax - zmin;
}