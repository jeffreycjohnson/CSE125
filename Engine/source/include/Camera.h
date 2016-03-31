#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include "ForwardDecs.h"
#include "Transform.h"
#include "Renderer.h"

class Camera : public Component
{
	private:
		float shakeAmount, startShakeAmount, shakeDuration, startShakeDuration;
		bool isShaking;
		float currentFOV, prevFOV, fovStartTime;
		glm::vec3 forward, up, position, prevPosition, velocity;
		glm::mat4 matrix;

	public:
        std::unique_ptr<Framebuffer> fbo;
		Transform offset;
		float fov, fovDuration;
        std::vector<std::unique_ptr<RenderPass>> passes;
        float width, height;
        Texture* renderResult;

		explicit Camera(int w = Renderer::getWindowWidth(), int h = Renderer::getWindowHeight(), bool defaultPasses = true,
            const std::vector<GLint>& colorFormats = { GL_RGBA8, GL_RGBA16, GL_RGBA16F, GL_RGBA16F });
        ~Camera();
		glm::mat4 getCameraMatrix();
		void update(float deltaTime) override;
		void screenShake(float amount, float duration);
		glm::vec3 getForward() const;
		glm::vec3 getVelocity() const;
		float getFOV() const;
};

#endif