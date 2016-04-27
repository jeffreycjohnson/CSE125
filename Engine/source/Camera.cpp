 #include "Camera.h"
#include "Timer.h"
#include "Renderer.h"
#include <gtc/matrix_inverse.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/compatibility.hpp>
#include <glm.hpp>
#include <time.h>
#include <iostream>
#include "GameObject.h"
#include "Sound.h"
#include "Framebuffer.h"
#include "RenderPass.h"
#include "Collision.h"
#include "NetworkManager.h"

Camera::Camera(int w, int h, bool defaultPasses, const std::vector<GLenum>& colorFormats) : width(w), height(h)
{
	currentFOV = prevFOV = fov = atan(1.f) * 4.0f / 3.0f;
	fovDuration = 1;
	offset.setPosition(0, 0, 0);
	up = {0, 1, 0};
	srand(time(NULL));
    fbo = std::make_unique<Framebuffer>(w, h, colorFormats, true);
    if (defaultPasses)
    {
        passes.push_back(std::make_unique<GBufferPass>());
        passes.push_back(std::make_unique<LightingPass>());
        passes.push_back(std::make_unique<SkyboxPass>(nullptr));
        passes.push_back(std::make_unique<ForwardPass>());
        passes.push_back(std::make_unique<ParticlePass>());
        passes.push_back(std::make_unique<BloomPass>());
    }
    Renderer::cameras.push_back(this);
}

Camera::~Camera()
{
    if(!Renderer::shutdown) Renderer::cameras.remove(this);
}

glm::mat4 Camera::getCameraMatrix()
{
	return glm::affineInverse(gameObject->transform.getTransformMatrix() * offset.getTransformMatrix());
}

void Camera::update(float deltaTime)
{
	if (fov != prevFOV)
	{
		fovStartTime = (float)Timer::time();
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

Ray Camera::getEyeRay() const
{
	return Ray(position, forward);
}

void Camera::setGameObject(GameObject * go)
{
	Component::setGameObject(go);
}

std::vector<char> Camera::serialize()
{
	CameraNetworkData cnd = CameraNetworkData(gameObject->getID());
	return structToBytes(cnd);
}

void Camera::deserializeAndApply(std::vector<char> bytes)
{
	CameraNetworkData cnd = structFromBytes<CameraNetworkData>(bytes);
	GameObject * player = GameObject::FindByID(cnd.objectID);
	std::cout << "received camera. Attaching to object " << cnd.objectID << std::endl;
	Renderer::mainCamera->setGameObject(player);
}

void Camera::Dispatch(const std::vector<char> &bytes, int messageType, int messageId)
{
	CameraNetworkData cnd = structFromBytes<CameraNetworkData>(bytes);
	GameObject *go = GameObject::FindByID(messageId);
	if (go == nullptr)
	{
		throw std::runtime_error("Cannot attach camera to nonexistant gameobject");
	}

	// Remove Camera from SceneRoot if already there.
	// Can remove this if we prevent game from crashing on startup due to lack of camera.
	Camera * mainCam = GameObject::SceneRoot.getComponent<Camera>();
	if (mainCam != nullptr) {
		GameObject::SceneRoot.removeComponent<Camera>(false);
	}

	Camera *goCamera = go->getComponent<Camera>();
	if (goCamera != nullptr)
	{
		// player already has a camera
		// change what it points to
		std::cout << "changing camera..." << std::endl;
		goCamera->deserializeAndApply(bytes);
	}
	else
	{
		// player doesn't have a camera
		// assign it the correct one
		std::cout << "goCamera is null. Adding MainCamera to object " << go->getID() << std::endl;
		go->addComponent(Renderer::mainCamera);
	}
}