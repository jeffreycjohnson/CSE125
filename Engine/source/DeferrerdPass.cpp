#include "RenderPass.h"
#include "GameObject.h"
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Framebuffer.h"
#include "Renderer.h"
#include <gtc/matrix_inverse.hpp>
#include "Camera.h"

const static glm::mat4 bias(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0);

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
        mesh->material->bind();
        mesh->draw();
    }
    CHECK_ERROR();
}


LightingPass::LightingPass()
{
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["colorTex"] = 0;
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["normalTex"] = 1;
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["posTex"] = 2;
}

void LightingPass::render(Camera* camera)
{
    Renderer::getShader(DEFERRED_SHADER_LIGHTING)->use();
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["uScreenSize"] = glm::vec2(camera->width, camera->height);
    glDepthMask(GL_FALSE);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
    glDrawBuffer(GL_COLOR_ATTACHMENT3);
    glClear(GL_COLOR_BUFFER_BIT);

    camera->fbo->bindTexture(0, 0);
    camera->fbo->bindTexture(1, 1);
    camera->fbo->bindTexture(2, 2);
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["colorTex"] = 0;
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["normalTex"] = 1;
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["posTex"] = 2;
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["shadowTex"] = 3;
    (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["uIV_Matrix"] = Renderer::currentCamera->gameObject->transform.getTransformMatrix();
    CHECK_ERROR();

    for(auto light : Renderer::renderBuffer.light) {
        auto d = dynamic_cast<DirectionalLight*>(light);
        if (!d)
        {
            glCullFace(GL_FRONT);
        }
        else
        {
            glCullFace(GL_BACK);
            if(d->shadowCaster && d->shadowMap->fbo)
            {
                d->shadowMap->fbo->bindDepthTexture(3);
                (*Renderer::getShader(DEFERRED_SHADER_LIGHTING))["uShadow_Matrix"] = bias * DirectionalLight::shadowMatrix * glm::affineInverse(d->gameObject->transform.getTransformMatrix());
            }
        }

        glDisable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDrawBuffer(GL_COLOR_ATTACHMENT3);

        light->deferredPass();
    }
    CHECK_ERROR();
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    auto& currentEntry = Mesh::meshMap["Plane"];

    if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
        glBindVertexArray(currentEntry.vaoHandle);
        Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
    }

    (*Renderer::currentShader)["uLightType"] = 3;
    (*Renderer::currentShader)["uScale"] = 1.f;
    (*Renderer::currentShader)["uLightPosition"] = glm::vec3(0);
    (*Renderer::currentShader)["uV_Matrix"] = glm::mat4();
    (*Renderer::currentShader)["uP_Matrix"] = glm::mat4();
    glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
    (*Renderer::currentShader)["uV_Matrix"] = Renderer::view;
    (*Renderer::currentShader)["uP_Matrix"] = Renderer::perspective;
    CHECK_ERROR();

    // TODO : Render Ambient
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
