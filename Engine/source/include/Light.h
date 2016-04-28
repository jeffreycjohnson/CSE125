#ifndef INCLUDE_LIGHT_H
#define INCLUDE_LIGHT_H

#include "ForwardDecs.h"
#include "Component.h"
#include <glm.hpp>
#include <string>

enum {
	is_light,
	is_pointlight,
	is_directionallight,
	is_spotlight
};

class Light : public Component
{
public:
	/*Light(glm::vec3 color, bool shadowCaster, float radius, float constantFalloff,
		float linearFalloff, float exponentialFalloff) :
		color(color), shadowCaster(shadowCaster), radius(radius), constantFalloff(constantFalloff),
		linearFalloff(linearFalloff), exponentialFalloff(exponentialFalloff) {};*/
	void setColor(glm::vec3 color);
	void setShadowCaster(bool shadowCaster);
	void setRadius(float radius);
	void setConstantFalloff(float constantFalloff);
	void setLinearFalloff(float linearFalloff);
	void setExponentialFalloff(float exponentialFalloff);

	glm::vec3 getColor();
	bool getShadowCaster();
	float getRadius();
	float getConstantFalloff();
	float getLinearFalloff();
	float getExponentialFalloff();

	void setGameObject(GameObject* object) override;

	static void Dispatch(const std::vector<char> &bytes, int messageType, int messageId);

    virtual void forwardPass(int index) = 0;
    virtual void deferredPass() = 0;

	void deserializeAndApply(std::vector<char> bytes) override;

	std::vector<char> serialize() override;

protected:
	glm::vec3 color;
	bool shadowCaster = false;
	float radius = 0.02f;
	// For now I'm ignoring this and hardcoding it to use the defaults
	float constantFalloff = 1, linearFalloff = 0, exponentialFalloff = 1;

    void deferredHelper(const std::string& meshName);
	void postToNetwork();

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
	//TODO: Robustnesssss, Angle, Exponent is vehemently not supported
    float angle = 30, exponent = 5;

    void forwardPass(int index) override;
    void deferredPass() override;

	//void deserializeAndApply(std::vector<char> bytes) override;
};

#endif
