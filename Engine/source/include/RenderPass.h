#ifndef INCLUDE_RENDERPASS_H
#define INCLUDE_RENDERPASS_H

#include "ForwardDecs.h"
#include "Skybox.h"

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void render(Camera*) = 0;
};

class ForwardPass : public RenderPass
{
public:
    void render(Camera* camera) override;
};

class ParticlePass : public RenderPass
{
public:
    void render(Camera* camera) override;
};

class GBufferPass : public RenderPass
{
public:
    void render(Camera* camera) override;
};

class LightingPass : public RenderPass
{
public:
    LightingPass();
    void render(Camera* camera) override;
};

class SkyboxPass : public RenderPass
{
public:
	explicit SkyboxPass(Skybox* skybox);
    void render(Camera* camera) override;
	Skybox* skybox;
};

class DirectionalShadowPass : public RenderPass
{
public:
    void render(Camera* camera) override;
};

class PointShadowPass : public RenderPass
{
public:
    void render(Camera* camera) override;
};

class DebugPass : public RenderPass
{
public:
	static bool drawLights;
	static bool drawColliders;
	static bool drawDynamicOctree;
	static bool drawStaticOctree;

	static glm::vec3 colliderColor;  // Draw colliders with this color by default
	static glm::vec3 collidingColor; // Draw colliders that are colliding in this color
	static glm::vec3 octreeColor;    // Draw octree nodes in this color

    void render(Camera* camera) override;
};

// Samples controls quality, radius the SSAO coverage
class SSAOPass : public RenderPass
{
public:
	explicit SSAOPass(unsigned int samples = 16, float radius = 0.5f);
	void render(Camera* camera) override;
    glm::vec3 ambientColor = glm::vec3(0.1f);

private:
	unsigned int samples;
	float radius;
	std::unique_ptr<Texture> noise;
	std::unique_ptr<Framebuffer> aoBuffer;
	std::vector<glm::vec3> sampleBuf;
};

class BloomPass : public RenderPass
{
public:
    BloomPass();
    ~BloomPass() override;
    void render(Camera* camera) override;

private:
    Framebuffer * brightPass;
    Framebuffer * blurBuffers[5];
};

class UIPass : public RenderPass
{
public:
	UIPass();
	~UIPass() override;
	void render(Camera* camera) override;

private:
	std::unique_ptr<Texture> tex;
};

#endif