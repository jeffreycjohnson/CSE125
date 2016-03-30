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

class ShadowPass : public RenderPass
{
public:
    void render(Camera* camera) override;
};

class DebugPass : public RenderPass
{
public:
    void render(Camera* camera) override;
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

#endif