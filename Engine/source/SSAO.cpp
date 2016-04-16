#include "RenderPass.h"
#include <random>
#include "Texture.h"
#include "Framebuffer.h"
#include "Camera.h"
#include "Shader.h"

SSAOPass::SSAOPass(unsigned int samples, float radius) : samples(samples), radius(radius)
{
	// make_unique wasn't working???
	std::vector<GLenum> format = { GL_RGB8 };
	aoBuffer = std::make_unique<Framebuffer>(Renderer::getWindowWidth(), Renderer::getWindowHeight(), format, false);
    // use the following if there are performance or vram problems (runs at 1/2 resolution)
    //aoBuffer = std::make_unique<Framebuffer>(Renderer::getWindowWidth()/2, Renderer::getWindowHeight()/2, format, false);

	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	// generate sample points
	for (unsigned int i = 0; i < samples; i++) {
		// generate random unit vector along hemisphere
		auto vec = glm::normalize(glm::vec3(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator)));
		// randomly scale it
		vec *= randomFloats(generator);
		// bias it towards 0, but never zero length
		vec *= pow(float(i) / samples, 2) * 0.9f + 0.1f;
		sampleBuf.push_back(vec);
	}

	// generate rotations of the sample points to reduce banding
	unsigned char noiseBuf[16 * 3];
	for (unsigned int i = 0; i < 16; i++) {
		noiseBuf[i * 3] = static_cast<unsigned char>(randomFloats(generator) * 255);
		noiseBuf[i * 3 + 1] = static_cast<unsigned char>(randomFloats(generator) * 255);
		noiseBuf[i * 3 + 2] = static_cast<unsigned char>(0.f);
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
	// pass random samples to an array in the shader
	for (unsigned int i = 0; i < samples; i++) {
		shader[std::string("uSamples[") + std::to_string(i) + "]"] = sampleBuf[i];
	}
	shader["uSampleCount"] = samples;
	shader["uRadius"] = radius;

	camera->fbo->bindTexture(0, 1);
	camera->fbo->bindTexture(1, 2);
	shader["normalTex"] = 0;
	shader["posTex"] = 1;

	noise->bindTexture(3);
	shader["rotationTex"] = 3;

	GLuint drawBuffer = GL_COLOR_ATTACHMENT0;
	aoBuffer->bind(1, &drawBuffer, false);
	aoBuffer->draw(); // first pass that fills the ao buffer

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	auto& blurShader = Renderer::getShader(SSAO_BLUR);
	blurShader.use();

	drawBuffer = GL_COLOR_ATTACHMENT3;
	camera->fbo->bind(1, &drawBuffer, false);

	aoBuffer->bindTexture(0, 0);
	blurShader["inputTex"] = 0;
	camera->fbo->bindTexture(1, 0);
	blurShader["colorTex"] = 1;
    blurShader["ambientColor"] = ambientColor;

	camera->fbo->draw(); // second pass that blurs and applies the ao
	glDisable(GL_BLEND);
}