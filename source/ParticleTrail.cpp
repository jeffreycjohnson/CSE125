#include "ParticleTrail.h"
#include "GameObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Material.h"

#define FLOAT_SIZE 4
#define VERTEX_ATTRIB_LOCATION 0
#define NORMAL_ATTRIB_LOCATION 1
#define DIST_ATTRIB_LOCATION 2
#define DIR_ATTRIB_LOCATION 3

void ParticleTrail::uploadData() {
	if (pointList.size() < 1) {
		return;
	}
	unsigned int arrayIndex = 0;
	float distance = 0;
	glm::vec3 lastPosition;
	for (TrailPoint& currentPoint : pointList) {
		distance += (arrayIndex == 0) ? 0 : 1;//glm::length(currentPoint.position - lastPosition);
		lastPosition = currentPoint.position;

		//add 2 points - one for each side of trail
		for (int corner : {1, -1}) {
			for (int dimen = 0; dimen < 3; ++dimen) {
				megaArray[arrayIndex++] = currentPoint.position[dimen];
			}
			for (int dimen = 0; dimen < 3; ++dimen) {
				megaArray[arrayIndex++] = currentPoint.normal[dimen];
			}
			megaArray[arrayIndex++] = distance;
			megaArray[arrayIndex++] = corner;
		}
	}

	if (arrayIndex > 2 * (elementStride * maxPoints)) throw "Out of range";

		glBindVertexArray(vaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, meshBuffer);
		glBufferData(GL_ARRAY_BUFFER, arrayIndex * sizeof(float), megaArray, GL_STREAM_DRAW);
		int stride = FLOAT_SIZE * (elementStride);
		uintptr_t currentOffset = 0;
		glVertexAttribPointer(VERTEX_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)0); currentOffset += (FLOAT_SIZE * 3);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (FLOAT_SIZE * 3);
		glVertexAttribPointer(DIST_ATTRIB_LOCATION, 1, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (FLOAT_SIZE * 1);
		glVertexAttribPointer(DIR_ATTRIB_LOCATION, 1, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (FLOAT_SIZE * 1);
}
ParticleTrail::ParticleTrail()
{
	glGenVertexArrays(1, &vaoHandle);
	glGenBuffers(1, &meshBuffer);
	glBindVertexArray(vaoHandle);
	glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION);
	glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
	glEnableVertexAttribArray(DIST_ATTRIB_LOCATION);
	glEnableVertexAttribArray(DIR_ATTRIB_LOCATION);

	megaArray = new float[2 * (elementStride * maxPoints)];
}


ParticleTrail::~ParticleTrail()
{
	delete[] megaArray;
}

void ParticleTrail::update(float dt) {
    deltaTime = dt;
}

void ParticleTrail::draw()
{
    currentDelayTime += deltaTime;
    if (currentDelayTime >= addDelayTime) {
        addPoint(gameObject->transform.getWorldPosition());
        uploadData();

        currentDelayTime -= addDelayTime;
    }

	if (Renderer::gpuData.vaoHandle != vaoHandle) {
		glBindVertexArray(vaoHandle);
		Renderer::gpuData.vaoHandle = vaoHandle;
	}

	glEnable(GL_BLEND);
	if (additive)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(false);
	
	if (material) material->bind();

	//TODO move
	Renderer::setModelMatrix(glm::mat4(1));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, pointList.size()*2);

	glDepthMask(true);
	glDisable(GL_BLEND);
}

void ParticleTrail::addPoint(glm::vec3 point)
{
	if (emitting)
	{
		glm::vec3 normal (1,0,0);

		if (pointList.size() > 0) {
			normal = point - pointList.front().position;
			if (glm::length(normal) < 0.001) {
				normal = pointList.front().normal;
			}
			else {
				normal = glm::normalize(normal);
			}
			if (isnan(normal.z)) throw;
		}

	
		TrailPoint newTrailPoint;
		newTrailPoint.position = point;
		newTrailPoint.normal = normal;
		pointList.push_front(newTrailPoint);
	}

	if (pointList.size() > maxPoints)
		removingPoints = true;

	if (removingPoints && pointList.size() > 0) {
		pointList.pop_back();
	}
}
