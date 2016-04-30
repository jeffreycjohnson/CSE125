#include "Collision.h"

RayHitInfo::RayHitInfo()
{
	hitTime = INFINITY;
	intersects = false;
	collider = nullptr;
}
