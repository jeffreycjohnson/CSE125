#ifndef INCLUDE_TRANSFORM_H
#define INCLUDE_TRANSFORM_H

#include "ForwardDecs.h"
#include <gtc/quaternion.hpp>
#include <vector>
#include "Component.h"

class Transform : public Component
{
        //Position - vector
        glm::vec3 position;
        //Rotation - quaternion
        glm::quat rotation;
        //Scale - vector
        glm::vec3 scaleFactor = glm::vec3(1, 1, 1);

        //dirty flag for Transform Matrix
        bool transformMatrixDirty = true;
        //Cached Transform Matrix
        glm::mat4 transformMatrix;
        //Cached other stuff
        glm::vec3 cachedWorldPos;
        bool worldPosDirty = true;
        float cachedWorldScale;
        bool worldScaleDirty = true;

        Transform* oldParent = nullptr;

	public:
		//parent Transform
		Transform* parent = nullptr;

        void setDirty();

		//child Transforms
		std::vector<Transform*> children;


		/**
		 * Translate
		 * -Transform Dirty
		 */
        void translate(float x, float y, float z);
        void translate(const glm::vec3& diff);
		void setPosition(float x, float y, float z);
		void setPosition(const glm::vec3& pos);


		/**
		 * Rotate
		 * -Transform Dirty
		 * -Normals Dirty
		 */
		void setRotate(const glm::quat& rotation);
        void rotate(const glm::quat& diff);

		/**
		 * Scale
		 */
		void setScale(const glm::vec3& scale);
        void scale(float s);


		/**
		 * Get Transform Matrix
		 * -uses parent's matrix as well
		 */
        glm::mat4 getTransformMatrix();
        glm::quat getRotation() const;
        glm::vec3 getPosition() const;
		glm::vec3 getWorldPosition();
        glm::vec3 getScale() const;
		float getWorldScale();
};

#endif
