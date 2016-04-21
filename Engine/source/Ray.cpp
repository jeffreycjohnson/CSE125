#include "Collision.h"

Ray::Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {
	direction = glm::normalize(direction);
	t = 0.0f;
}

// Returns a discrete point along the ray at the timestep t
glm::vec3 Ray::getCurrentPosition() const {
	return origin + t * direction;
}
glm::vec3 Ray::getPos(float tt) const {
	return origin + tt * direction;
}