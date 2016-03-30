#include "Transform.h"
#include <gtc/matrix_transform.hpp>

void Transform::setDirty()
{
    transformMatrixDirty = true;
    worldPosDirty = true;
    worldScaleDirty = true;
    for (auto child : children) child->setDirty();
}

/**
* Translate
* -Transform Dirty
*/
void Transform::translate(float x, float y, float z) {
    setDirty();
    position += glm::vec3(x, y, z);
}

void Transform::translate(const glm::vec3& diff) {
    setDirty();
    position += diff;
}

void Transform::setPosition(float x, float y, float z) {
    setDirty();
	position = glm::vec3(x, y, z);
}

void Transform::setPosition(const glm::vec3& pos) {
    setDirty();
	position = pos;
}


/**
* Rotate
* -Transform Dirty
* -Normals Dirty
*/
void Transform::rotate(const glm::quat& diff) {
    setDirty();
    rotation *= diff;
}

void Transform::setRotate(const glm::quat& diff) {
    setDirty();
	rotation = glm::quat();
	rotate(diff);
}

/**
* Scale
*/
void Transform::setScale(const glm::vec3& scale) {
    setDirty();
	scaleFactor = scale;
}

void Transform::scale(float s) {
    setDirty();
    scaleFactor *= s;
}


/**
* Get Transform Matrix
* -uses parent's matrix as well
*/
glm::mat4 Transform::getTransformMatrix() {
    if (transformMatrixDirty || parent != oldParent) {
        transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::translate(transformMatrix, position);
        transformMatrix *= glm::mat4_cast(rotation);
        transformMatrix = glm::scale(transformMatrix, scaleFactor);
        glm::mat4 parMat = (parent) ? parent->getTransformMatrix() : glm::mat4(1.f);
        transformMatrix = parMat * transformMatrix;
        transformMatrixDirty = false;
        oldParent = parent;
    }
    return transformMatrix;
}

glm::quat Transform::getRotation() const
{
    return rotation;
}

glm::vec3 Transform::getPosition() const
{
    return position;
}

glm::vec4 originPoint(0, 0, 0, 1);

glm::vec3 Transform::getWorldPosition() {
    if(worldPosDirty)
    {
        cachedWorldPos = glm::vec3(getTransformMatrix() * originPoint);
        worldPosDirty = false;
    }
	return cachedWorldPos;
}

glm::vec3 Transform::getScale() const
{
    return scaleFactor;
}

float Transform::getWorldScale() {
    if(worldScaleDirty)
    {
        cachedWorldScale = glm::length(glm::vec3(getTransformMatrix()[0]));
        worldScaleDirty = false;
    }
    return cachedWorldScale;
}