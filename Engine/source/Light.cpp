#include "Light.h"
#include "Mesh.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Framebuffer.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/matrix_inverse.hpp>
#include "Camera.h"
#include "RenderPass.h"
#include "NetworkManager.h"
#include "NetworkUtility.h"
#include <iostream>

glm::mat4 DirectionalLight::shadowMatrix = glm::ortho<float>(-25, 25, -25, 25, -50, 50);

void Light::setColor(glm::vec3 color)
{
	this->color = color;

	postToNetwork();
}

void Light::setShadowCaster(bool shadowCaster)
{
	this->shadowCaster = shadowCaster;

	postToNetwork();

}

void Light::setRadius(float radius)
{
	this->radius = radius;

	postToNetwork();
}

void Light::setConstantFalloff(float constantFalloff)
{
	this->constantFalloff = constantFalloff;

	postToNetwork();
}

void Light::setLinearFalloff(float linearFalloff)
{
	this->linearFalloff = linearFalloff;

	postToNetwork();
}

void Light::setExponentialFalloff(float exponentialFalloff)
{
	this->exponentialFalloff = exponentialFalloff;

	postToNetwork();
}

glm::vec3 Light::getColor()
{
	return this->color;
}

bool Light::getShadowCaster()
{
	return this->shadowCaster;
}

float Light::getRadius()
{
	return this->radius;
}

float Light::getConstantFalloff()
{
	return this->constantFalloff;
}

float Light::getLinearFalloff()
{
	return this->linearFalloff;
}

float Light::getExponentialFalloff()
{
	return this->exponentialFalloff;
}

void Light::setGameObject(GameObject * object)
{
	Component::setGameObject(object);
	postToNetwork();
}

void Light::Dispatch(const std::vector<char> &bytes, int messageType, int messageId) {
	LightNetworkData lnd = structFromBytes<LightNetworkData>(bytes);
	PointLight *gopointlight;
	DirectionalLight *godirectionallight;
	SpotLight *gospotlight;

	GameObject *go = GameObject::FindByID(messageId);
	if (go == nullptr)
	{
		throw std::runtime_error("From Light.cpp/Dispatch: Nonexistant gameobject");
	}
	switch (lnd.lightType) {
	case is_light:
		throw std::runtime_error("From Light.cpp/Dispatch: This should not happen!\n Lights need to be specified...");
		break;
	case is_pointlight:
		gopointlight = go->getComponent<PointLight>();
		if (gopointlight != nullptr) {
			gopointlight->deserializeAndApply(bytes);
		}
		else {
			gopointlight = new PointLight();
			go->addComponent(gopointlight);
			gopointlight->deserializeAndApply(bytes);
		}
		break;
	case is_directionallight:
		godirectionallight = go->getComponent<DirectionalLight>();
		if (godirectionallight != nullptr) {
			godirectionallight->deserializeAndApply(bytes);
		}
		else {
			godirectionallight = new DirectionalLight();
			go->addComponent(godirectionallight);
			godirectionallight->deserializeAndApply(bytes);
		}
		break;
	case is_spotlight:
		throw std::runtime_error("From Light.cpp/Dispatch: SpotlightNotImplementd");
		gospotlight = go->getComponent<SpotLight>();
		if (gospotlight != nullptr) {
			gospotlight->deserializeAndApply(bytes);
		}
		else {
			gospotlight = new SpotLight();
			go->addComponent(gospotlight);
			gospotlight->deserializeAndApply(bytes);
		}
		break;
	default:
		throw std::runtime_error("From Light.cpp/Dispatch: out of enum? Impossible");
		break;
	}
}

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

void Light::postToNetwork()
{
	if (NetworkManager::getState() != SERVER_MODE) return;

	GameObject *my = gameObject;
	if (my == nullptr)
	{
		std::cerr << "Light ain't got no attached game object modified??" << std::endl;
		return;
	}

	std::cout << "Sending light..." << std::endl;
	NetworkManager::PostMessage(serialize(), LIGHT_NETWORK_DATA, my->getID());
}

void PointLight::forwardPass(int index)
{
	for (int shaderId : Renderer::shaderForwardLightList) {
		Renderer::getShader(shaderId)["uLightData[" + std::to_string(3*index) + "]"] = glm::vec4(gameObject->transform.getWorldPosition(), 1.0);
		Renderer::getShader(shaderId)["uLightData[" + std::to_string(3*index+1) + "]"] = glm::vec4(color, 1);
        Renderer::getShader(shaderId)["uLightData[" + std::to_string(3*index+2) + "]"] = glm::vec4(constantFalloff, linearFalloff, exponentialFalloff, 1);
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

void Light::deserializeAndApply(std::vector<char> bytes)
{
	LightNetworkData lnd = structFromBytes<LightNetworkData>(bytes);

	setColor(glm::vec3(lnd.colorr, lnd.colorg, lnd.colorb));
	setShadowCaster(lnd.shadowCaster);
	setRadius(lnd.radius);
	setConstantFalloff(lnd.constantFalloff);
	setLinearFalloff(lnd.linearFalloff);
	setExponentialFalloff(lnd.exponentialFalloff);
}

std::vector<char> Light::serialize()
{
	int lightType;
	Light * light_t;
	PointLight * pointlight_t;
	DirectionalLight * directionallight_t;
	SpotLight * spotlight_t;

	if (pointlight_t = dynamic_cast<PointLight*>(this)) {
		lightType = is_pointlight;
	}
	else if (directionallight_t = dynamic_cast<DirectionalLight*>(this)) {
		lightType = is_directionallight;
	}
	else if (spotlight_t = dynamic_cast<SpotLight*>(this)) {
		lightType = is_spotlight;
	}
	else if (light_t = dynamic_cast<Light*>(this)) {
		lightType = is_light;
		std::cerr << "This should not happen" << std::endl;
	}

	LightNetworkData lnd = LightNetworkData(
		gameObject->getID(),
		lightType,
		getColor(),
		getShadowCaster(),
		getRadius(),
		getConstantFalloff(),
		getLinearFalloff(),
		getExponentialFalloff());
	std::cout << "My Lighttype... " << lightType << std::endl;
	return structToBytes(lnd);
}

DirectionalLight::DirectionalLight(bool shadow)
{
    if(shadow)
    {
        shadowCaster = shadow;
		shadowMap = std::make_unique<Camera>(2048, 2048, false, std::vector<GLenum>({}));
        shadowMap->passes.push_back(std::make_unique<ShadowPass>());
        shadowMap->setGameObject(gameObject);
    }
}

void DirectionalLight::forwardPass(int index)
{
	for (int shaderId : Renderer::shaderForwardLightList) {
		Renderer::getShader(shaderId)["uLightData[" + std::to_string(2 * index) + "]"] = glm::vec4(gameObject->transform.getTransformMatrix() * glm::vec4(0, 0, 1, 0));
		Renderer::getShader(shaderId)["uLightData[" + std::to_string(2 * index + 1) + "]"] = glm::vec4(color, 0);
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
        Renderer::getShader(SHADOW_SHADER_ANIM)["uV_Matrix"] = mat;
        Renderer::getShader(SHADOW_SHADER)["uV_Matrix"] = mat;
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
