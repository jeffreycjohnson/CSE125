#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include "ForwardDecs.h"
#include "Transform.h"
#include "Renderer.h"

const std::vector<GLenum> defaultFormats{ GL_RGBA8, GL_RGBA16, GL_RGBA16F, GL_RGBA16F };

class Camera : public Component
{
	protected:
		float shakeAmount = 0.f, startShakeAmount = 0.f, shakeDuration = 0.f, startShakeDuration = 0.f;
		bool isShaking = false;
		float currentFOV, prevFOV, fovStartTime;
		glm::vec3 forward, up, position, prevPosition, velocity;
		glm::mat4 matrix;

        Camera();

	public:
        std::unique_ptr<Framebuffer> fbo;
		Transform offset;
		float fov, fovDuration;
        std::vector<std::unique_ptr<RenderPass>> passes;
        float width, height;
        Texture* renderResult;

		explicit Camera(int w, int h, bool defaultPasses = true,
			const std::vector<GLenum>& colorFormats = defaultFormats);
        virtual ~Camera();
		glm::mat4 getCameraMatrix();
		void update(float deltaTime) override;
		void screenShake(float amount, float duration);
		glm::vec3 getForward() const;
		glm::vec3 getVelocity() const;
		float getFOV() const;
		Ray getEyeRay() const;

		void setGameObject(GameObject* go) override;

		std::vector<char> serialize();

		void deserializeAndApply(std::vector<char> bytes);

		static void Dispatch(const std::vector<char>& bytes, int messageType, int messageId);
};

class SpericalCamera : public Camera
{
public:
    SpericalCamera(int w, int h, bool defaultPasses = true,
        const std::vector<GLenum>& colorFormats = defaultFormats);
};

#endif