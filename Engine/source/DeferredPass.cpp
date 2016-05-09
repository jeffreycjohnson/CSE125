#include "RenderPass.h"
#include "GameObject.h"
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Framebuffer.h"
#include "Renderer.h"
#include <gtc/matrix_inverse.hpp>
#include "Camera.h"

void GBufferPass::render(Camera* camera)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_STENCIL_TEST);

    GLuint buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    camera->fbo->bind(3, buffers);
    for (auto mesh : Renderer::renderBuffer.deferred)
    {
        mesh->getMaterial()->bind();
        mesh->draw();
    }
    CHECK_ERROR();
}


LightingPass::LightingPass()
{
    Renderer::getShader(DEFERRED_SHADER_LIGHTING)["colorTex"] = 0;
    Renderer::getShader(DEFERRED_SHADER_LIGHTING)["normalTex"] = 1;
    Renderer::getShader(DEFERRED_SHADER_LIGHTING)["posTex"] = 2;
}

void LightingPass::render(Camera* camera)
{
    auto& shader = Renderer::getShader(DEFERRED_SHADER_LIGHTING);
    shader.use();
    shader["uScreenSize"] = glm::vec2(camera->width, camera->height);
    glDepthMask(GL_FALSE);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
    glDrawBuffer(GL_COLOR_ATTACHMENT3);
    glClear(GL_COLOR_BUFFER_BIT);

    camera->fbo->bindTexture(0, 0);
    camera->fbo->bindTexture(1, 1);
    camera->fbo->bindTexture(2, 2);
    shader["colorTex"] = 0;
    shader["normalTex"] = 1;
    shader["posTex"] = 2;
    shader["shadowTex"] = 3;
    shader["pointShadowTex"] = 4;
    shader["lightGradientTex"] = 5;
    shader["uIV_Matrix"] = Renderer::currentCamera->gameObject->transform.getTransformMatrix();
    CHECK_ERROR();

    for(auto light : Renderer::renderBuffer.light) {
        auto d = dynamic_cast<DirectionalLight*>(light);
        if (!d) glCullFace(GL_FRONT);
        else glCullFace(GL_BACK);

        glDisable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDrawBuffer(GL_COLOR_ATTACHMENT3);
        CHECK_ERROR();

        light->deferredPass();
    }
    CHECK_ERROR();
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    auto& currentEntry = Mesh::meshMap["assets/Primatives.obj/Plane"];

    if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
        glBindVertexArray(currentEntry.vaoHandle);
        Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
    }

    shader["uLightType"] = 3;
    shader["uScale"] = 1.f;
    shader["uLightPosition"] = glm::vec3(0);
    shader["uV_Matrix"] = glm::mat4();
    shader["uP_Matrix"] = glm::mat4();
    glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
    shader["uV_Matrix"] = Renderer::view;
    shader["uP_Matrix"] = Renderer::perspective;
    CHECK_ERROR();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
