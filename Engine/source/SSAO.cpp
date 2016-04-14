#include "RenderPass.h"
#include <random>
#include "Texture.h"
#include "Framebuffer.h"
#include "Camera.h"
#include "Shader.h"

SSAOPass::SSAOPass(unsigned int samples, float radius) : samples(samples), radius(radius)
{
	// make_unique wasn't working???
	std::vector<GLenum> format = { GL_R8 };
	aoBuffer = std::make_unique<Framebuffer>(Renderer::getWindowWidth(), Renderer::getWindowHeight(), format, false);

	auto& shader = Renderer::getShader(SSAO_SHADER);

	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	// generate sample points
	for (int i = 0; i < samples; i++) {
		// generate random unit vector along hemisphere
		auto vec = glm::normalize(glm::vec3(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator)));
		// randomly scale it
		vec *= randomFloats(generator);
		// bias it towards 0, but never zero length
		vec *= pow(float(i) / samples, 2) * 0.9f + 0.1f;
		sampleBuf.push_back(vec);
	}

	// generate rotations of the sample points to reduce banding
	char noiseBuf[16 * 3];
	for (int i = 0; i < 16; i++) {
		noiseBuf[i * 3] = static_cast<char>((randomFloats(generator) * 2 - 1) * 255);
		noiseBuf[i * 3 + 1] = static_cast<char>((randomFloats(generator) * 2 - 1) * 255);
		noiseBuf[i * 3 + 2] = static_cast<char>(0.f);
	}
	noise = std::make_unique<Texture>(noiseBuf, 4, 4, GL_RGB);
}

void SSAOPass::render(Camera* camera)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);

	auto& shader = Renderer::getShader(SSAO_SHADER);
	shader.use();
	// pass samples to an array in the shader
	for (int i = 0; i < samples; i++) {
		shader[std::string("uSamples[") + std::to_string(i) + "]"] = sampleBuf[i];
	}
	shader["uSampleCount"] = samples;
	shader["uRadius"] = radius;

	camera->fbo->bindTexture(1, 0);
	camera->fbo->bindTexture(2, 1);
	shader["normalTex"] = 0;
	shader["posTex"] = 1;

	noise->bindTexture(2);
	shader["rotationTex"] = 2;

	GLuint drawBuffer = GL_COLOR_ATTACHMENT3;
	camera->fbo->bind(1, &drawBuffer, false);
	camera->fbo->draw();
	/*
	GLuint drawBuffer = GL_COLOR_ATTACHMENT0;
	aoBuffer->bind(1, &drawBuffer, false);
	aoBuffer->draw();
	*/
}