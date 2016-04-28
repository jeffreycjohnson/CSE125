#include "RenderPass.h"
#include "GameObject.h"
#include "Light.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Camera.h"
#include "Material.h"
#include "Collision.h"

void ForwardPass::render(Camera* camera) {
	unsigned int lightIndex = 0;
	//TODO sort lights by importance?
	for (auto l : Renderer::renderBuffer.light) {
		if (lightIndex > FORWARD_SHADER_LIGHT_MAX) break;
		l->forwardPass(lightIndex++);
	}
	for (auto mesh : Renderer::renderBuffer.forward) {
        if(mesh->material->shader == &Renderer::getShader(FORWARD_UNLIT) || mesh->material->shader == &Renderer::getShader(FORWARD_EMISSIVE)) glDepthMask(GL_FALSE);
        mesh->material->bind();
		mesh->draw();
        if (mesh->material->shader == &Renderer::getShader(FORWARD_UNLIT) || mesh->material->shader == &Renderer::getShader(FORWARD_EMISSIVE)) glDepthMask(GL_TRUE);
	}
}

void ParticlePass::render(Camera* camera) {
    for (auto mesh : Renderer::renderBuffer.particle) {
        mesh->draw();
    }
}

// All of these options are overrideable by config/options.ini
bool DebugPass::drawColliders = false;
bool DebugPass::drawLights = false;
bool DebugPass::drawDynamicOctree = false;
bool DebugPass::drawStaticOctree = false;
glm::vec3 DebugPass::colliderColor = glm::vec3(1, 1, 1); // White
glm::vec3 DebugPass::collidingColor = glm::vec3(1, 0, 0); // Red
glm::vec3 DebugPass::octreeColor = glm::vec3(1, 1, 0); // Yellow

void DebugPass::render(Camera* camera) {
	if (Renderer::drawDebug) {
		GameObject::SceneRoot.debugDraw();
	}
}

void DirectionalShadowPass::render(Camera* camera)
{
    auto l = camera->gameObject->getComponent<DirectionalLight>();
    if (!l || !l->getShadowCaster()) return;
    l->bindShadowMap();

    Renderer::getShader(SHADOW_SHADER)["uP_Matrix"] = DirectionalLight::shadowMatrix;
    Renderer::getShader(SHADOW_SHADER_ANIM)["uP_Matrix"] = DirectionalLight::shadowMatrix;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_STENCIL_TEST);
    glDrawBuffer(GL_NONE);

    for (auto mesh : Renderer::renderBuffer.deferred) {
        Material* mat = mesh->material;
        Shader* s = nullptr;
        if (mat->shader == &Renderer::getShader(DEFERRED_PBR_SHADER_ANIM)) s = &Renderer::getShader(SHADOW_SHADER_ANIM);
        else s = &Renderer::getShader(SHADOW_SHADER);
        s->use();
        mesh->draw();
    }
}

void PointShadowPass::render(Camera* camera)
{
    auto l = camera->gameObject->getComponent<PointLight>();
    if (!l || !l->getShadowCaster()) return;
    l->bindShadowMap();
    Renderer::getShader(SHADOW_CUBE_SHADER).use();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_STENCIL_TEST);
    glDrawBuffer(GL_NONE);
    CHECK_ERROR();

    for (auto mesh : Renderer::renderBuffer.deferred) {
        mesh->draw();
    }
    CHECK_ERROR();
}