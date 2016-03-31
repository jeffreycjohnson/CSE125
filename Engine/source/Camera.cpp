#include "Camera.h"
#include "Timer.h"
#include "Renderer.h"
#include <gtc/matrix_inverse.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/compatibility.hpp>
#include <glm.hpp>
#include <time.h>
#include "GameObject.h"
#include "Sound.h"
#include "Framebuffer.h"
#include "RenderPass.h"

Camera::Camera(int w, int h, bool defaultPasses, const std::vector<GLint>& colorFormats) : width(w), height(h)
{
	currentFOV = fov = atan(1.f) * 4.0f / 3.0f;
	fovDuration = 1;
	offset.setPosition(0, 0, 0);
	up = {0, 1, 0};
	srand(time(NULL));
    fbo = std::make_unique<Framebuffer>(w, h, colorFormats, true);
    if (defaultPasses)
    {
        passes.push_back(std::make_unique<GBufferPass>());
        passes.push_back(std::make_unique<LightingPass>());
        // TODO : get actual skybox
        passes.push_back(std::make_unique<SkyboxPass>(nullptr));
        passes.push_back(std::make_unique<ForwardPass>());
        passes.push_back(std::make_unique<ParticlePass>());
        passes.push_back(std::make_unique<BloomPass>());
    }
    Renderer::cameras.push_back(this);
}

Camera::~Camera()
{
    Renderer::cameras.remove(this);
}

glm::mat4 Camera::getCameraMatrix()
{
	//glm::affineInverse seems to have a bug with inverting scales atm (fixed in git, but not in lastest stable release), so we need to remove the scale manually
	
	matrix = gameObject->transform.getTransformMatrix() * offset.getTransformMatrix();
	glm::vec3 scale;
	scale[0] = 1/glm::length(glm::vec3(matrix[0][0], matrix[1][0], matrix[2][0]));
	scale[1] = 1/glm::length(glm::vec3(matrix[0][1], matrix[1][1], matrix[2][1]));
	scale[2] = 1/glm::length(glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]));
	return glm::affineInverse(glm::scale(matrix, scale));
}

void Camera::update(float deltaTime)
{
	if (fov != prevFOV)
	{
		fovStartTime = Timer::time();
		prevFOV = fov;
	}

	currentFOV = glm::lerp(currentFOV, fov, (float) (Timer::time() - fovStartTime) / fovDuration);

	if (isShaking)
	{
		float x = (float)rand() / RAND_MAX * shakeAmount - shakeAmount / 2.0f;
		float y = (float)rand() / RAND_MAX * shakeAmount - shakeAmount / 2.0f;
		float z = (float)rand() / RAND_MAX * shakeAmount - shakeAmount / 2.0f;
		offset.setPosition(x, y, z);

		shakeAmount = startShakeAmount * shakeDuration / startShakeDuration;
		shakeDuration -= deltaTime;

		if (shakeDuration <= 0)
		{
			shakeDuration = 0;
			shakeAmount = 0;
			isShaking = false;
			offset.setPosition(0, 0, 0);
		}
	}

	forward = { Renderer::view[0][2], Renderer::view[1][2], Renderer::view[2][2] };
	position = { matrix[3][0], matrix[3][1], matrix[3][2] }; // Has to be world space to work with forward vector
	velocity = position - prevPosition;
	prevPosition = position;

	// Update info for FMOD
	FMOD_VECTOR pos = { position.x, position.y, position.z };
	FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
	FMOD_VECTOR fwd = { forward.x, forward.y, forward.z };
	FMOD_VECTOR upv = { up.x, up.y, up.z };
	Sound::system->set3DListenerAttributes(0, &pos, &vel, &fwd, &upv);
}

void Camera::screenShake(float amount, float duration)
{
	if (amount >= shakeAmount)
	{
		shakeAmount = startShakeAmount = amount;
		shakeDuration = startShakeDuration = duration;
		isShaking = true;
	}
}

glm::vec3 Camera::getForward() const
{
	return -forward;
}

glm::vec3 Camera::getVelocity() const
{
	return velocity;
}

float Camera::getFOV() const
{
	return currentFOV;
}