#include "Light.h"
#include "Mesh.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Framebuffer.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/matrix_inverse.hpp>
#include "Camera.h"
#include "RenderPass.h"

glm::mat4 DirectionalLight::shadowMatrix = glm::ortho<float>(-25, 25, -25, 25, -50, 50);

void Light::deferredHelper(const std::string& meshName)
{
    auto& currentEntry = Mesh::meshMap[meshName];

    if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
        glBindVertexArray(currentEntry.vaoHandle);
        Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
    }

    (*Renderer::currentShader)["uLightFalloff"] = glm::vec3(constantFalloff, linearFalloff, exponentialFalloff);
    (*Renderer::currentShader)["uLightPosition"] = gameObject->transform.getWorldPosition();
    (*Renderer::currentShader)["uLightColor"] = color;
    (*Renderer::currentShader)["uLightSize"] = radius;
    (*Renderer::currentShader)["uLightDirection"] = glm::vec3(gameObject->transform.getTransformMatrix() * glm::vec4(0, 0, 1, 0));

    glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
    CHECK_ERROR();
}

void PointLight::forwardPass(int index)
{
	for (int shaderId : Renderer::shaderForwardLightList) {
		(*Renderer::getShader(shaderId))["uLightData[" + std::to_string(3*index) + "]"] = glm::vec4(gameObject->transform.getWorldPosition(), 1.0);
		(*Renderer::getShader(shaderId))["uLightData[" + std::to_string(3*index+1) + "]"] = glm::vec4(color, 1);
        (*Renderer::getShader(shaderId))["uLightData[" + std::to_string(3*index+2) + "]"] = glm::vec4(constantFalloff, linearFalloff, exponentialFalloff, 1);
	}
}

void PointLight::deferredPass()
{
    (*Renderer::currentShader)["uLightType"] = 0;
    auto max = std::max(std::max(color.r, color.g), color.b);
    float scale = (-linearFalloff + sqrtf(linearFalloff * linearFalloff - 4.0f * (constantFalloff - 256.0f * max) * exponentialFalloff))
        / (2.0f * exponentialFalloff);
    (*Renderer::currentShader)["uScale"] = scale;
    deferredHelper("Sphere");
}

void PointLight::debugDraw()
{
    auto max = std::max(std::max(color.r, color.g), color.b);
    float scale = (-linearFalloff + sqrtf(linearFalloff * linearFalloff - 4.0f * (constantFalloff - 256.0f * max) * exponentialFalloff))
        / (2.0f * exponentialFalloff);
    Renderer::drawSphere(glm::vec3(0), scale, glm::vec4(color, 1), &gameObject->transform);
}

DirectionalLight::DirectionalLight(bool shadow)
{
    if(shadow)
    {
        shadowCaster = shadow;
        shadowMap = std::make_unique<Camera>(2048, 2048, false, std::vector<GLint>({}));
        shadowMap->passes.push_back(std::make_unique<ShadowPass>());
        shadowMap->setGameObject(gameObject);
    }
}

void DirectionalLight::forwardPass(int index)
{
	for (int shaderId : Renderer::shaderForwardLightList) {
		(*Renderer::getShader(shaderId))["uLightData[" + std::to_string(2 * index) + "]"] = glm::vec4(gameObject->transform.getTransformMatrix() * glm::vec4(0, 0, 1, 0));
		(*Renderer::getShader(shaderId))["uLightData[" + std::to_string(2 * index + 1) + "]"] = glm::vec4(color, 0);
	}
}

void DirectionalLight::deferredPass()
{
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    (*Renderer::currentShader)["uLightType"] = 1;
    (*Renderer::currentShader)["uScale"] = 1.f;
    (*Renderer::currentShader)["uLightPosition"] = glm::vec3(0);
    (*Renderer::currentShader)["uV_Matrix"] = glm::mat4();
    (*Renderer::currentShader)["uP_Matrix"] = glm::mat4();
    deferredHelper("Plane");
    (*Renderer::currentShader)["uV_Matrix"] = Renderer::view;
    (*Renderer::currentShader)["uP_Matrix"] = Renderer::perspective;
}

void DirectionalLight::bindShadowMap()
{
    if(shadowMap->fbo && shadowCaster)
    {
        shadowMap->fbo->bind(0, nullptr);
        auto mat = glm::affineInverse(gameObject->transform.getTransformMatrix());
        (*Renderer::getShader(SHADOW_SHADER_ANIM))["uV_Matrix"] = mat;
        (*Renderer::getShader(SHADOW_SHADER))["uV_Matrix"] = mat;
    }
}

void DirectionalLight::update(float)
{
    gameObject->transform.translate(Renderer::mainCamera->gameObject->transform.getWorldPosition() - gameObject->transform.getWorldPosition());
}

void DirectionalLight::setGameObject(GameObject* object)
{
    Component::setGameObject(object);
    if (shadowCaster) shadowMap->setGameObject(object);
}

void SpotLight::forwardPass(int index)
{
}

void SpotLight::deferredPass()
{
}
