#ifndef INCLUDE_LIGHT_H
#define INCLUDE_LIGHT_H

#include "ForwardDecs.h"
#include "Component.h"
#include <glm.hpp>

class Light : public Component
{
public:
    glm::vec3 color;
    bool shadowCaster = false;
    float radius = 0.02f;
    // For now I'm ignoring this and hardcoding it to use the defaults
    float constantFalloff = 1, linearFalloff = 0, exponentialFalloff = 1;

    virtual void forwardPass(int index) = 0;
    virtual void deferredPass() = 0;

protected:
    void deferredHelper(const std::string& meshName);
};

class PointLight : public Light
{
public:
    void forwardPass(int index) override;
    void deferredPass() override;
    void debugDraw() override;
};

class DirectionalLight : public Light
{
public:
    explicit DirectionalLight(bool shadow = false);
    void forwardPass(int index) override;
    void deferredPass() override;
    void bindShadowMap();
    void update(float) override;
    void setGameObject(GameObject* object) override;

    std::unique_ptr<Camera> shadowMap;
    static glm::mat4 shadowMatrix;
};

class SpotLight : public Light
{
public:
    float angle = 30, exponent = 5;

    void forwardPass(int index) override;
    void deferredPass() override;
};

#endif
