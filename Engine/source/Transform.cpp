#include "Transform.h"
#include <gtc/matrix_transform.hpp>

#include "GameObject.h"
#include "NetworkUtility.h"

void Transform::Dispatch(const std::vector<char> &bytes, int messageType, int messageId)
{
	TransformNetworkData tnd = structFromBytes<TransformNetworkData>(bytes);

	GameObject *go = GameObject::FindByID(messageId);
	assert(go != nullptr);

	go->transform.deserializeAndApply(bytes);
}

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
	TransformNetworkData tnd = TransformNetworkData(
		gameObject->getID(),
		parent != nullptr ? parent->gameObject->getID() : -1,
		position,
		rotation,
		scaleFactor);

	return structToBytes(tnd);
}

void Transform::deserializeAndApply(std::vector<char> bytes)
{
	TransformNetworkData tnd = structFromBytes<TransformNetworkData>(bytes);

	setPosition(tnd.px, tnd.py, tnd.pz);
	setRotate(glm::quat(tnd.qw, tnd.qx, tnd.qy, tnd.qz));
	setScale(glm::vec3(tnd.sx, tnd.sy, tnd.sz));

	int myParentID = this->parent != nullptr ? parent->gameObject->getID() : -1;
	if (tnd.parentID != myParentID)
	{
		// if I have an actual parent to emancipate from
		if (myParentID != -1)
		{
			parent->gameObject->removeChild(this->gameObject);
		}

		// now, if we have an actual parent to adopt me
		if (tnd.parentID != -1)
		{
			GameObject *parentGO = GameObject::FindByID(tnd.parentID);
			assert(parentGO != nullptr);

			parentGO->addChild(this->gameObject);
		}
	}
}
