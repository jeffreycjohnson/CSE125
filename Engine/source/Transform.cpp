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
	rotation = glm::quat(diff);
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

#include <iostream>

// serialization
std::vector<char> Transform::serialize()
{
	TransformNetworkData tnd;

	tnd.transformID = this->componentID;

	tnd.px = position.x;
	tnd.py = position.y;
	tnd.pz = position.z;

	tnd.qw = rotation.w;
	tnd.qx = rotation.x;
	tnd.qy = rotation.y;
	tnd.qz = rotation.z;

	tnd.sx = scaleFactor.x;
	tnd.sy = scaleFactor.y;
	tnd.sz = scaleFactor.z;
	
	std::vector<char> bytes;
	bytes.resize(sizeof(tnd));
	memcpy(bytes.data(), &tnd, sizeof(tnd));

	return bytes;
}

void Transform::deserializeAndApply(std::vector<char> bytes)
{
	TransformNetworkData tnd = *((TransformNetworkData*)bytes.data());

	setPosition(tnd.px, tnd.py, tnd.pz);
	setRotate(glm::quat(tnd.qw, tnd.qx, tnd.qy, tnd.qz));
	setScale(glm::vec3(tnd.sx, tnd.sy, tnd.sz));
}
