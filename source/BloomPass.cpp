#include "RenderPass.h"
#include "Renderer.h"
#include "Framebuffer.h"
#include "Camera.h"
#include <glfw3.h>

extern GLFWwindow * mainWindow;

BloomPass::BloomPass()
{
    brightPass = new Framebuffer(Renderer::getWindowWidth(), Renderer::getWindowHeight(), 1, false, true);
    blurBuffers[0] = new Framebuffer(Renderer::getWindowWidth()  / 2, Renderer::getWindowHeight() / 2, 2, false, true);
    blurBuffers[1] = new Framebuffer(Renderer::getWindowWidth() / 4, Renderer::getWindowHeight() / 4, 2, false, true);
    blurBuffers[2] = new Framebuffer(Renderer::getWindowWidth()/ 8, Renderer::getWindowHeight() / 8, 2, false, true);
    blurBuffers[3] = new Framebuffer(Renderer::getWindowWidth() / 16, Renderer::getWindowHeight() / 16, 2, false, true);
    blurBuffers[4] = new Framebuffer(Renderer::getWindowWidth() / 32, Renderer::getWindowHeight() / 32, 2, false, true);
}

BloomPass::~BloomPass()
{
    delete brightPass;
    delete blurBuffers[0];
    delete blurBuffers[1];
    delete blurBuffers[2];
    delete blurBuffers[3];
    delete blurBuffers[4];
}

// TODO : If this needs better performance, blend smaller layers
// into the next one up to reduce texture reads
void BloomPass::render(Camera* camera)
{
    auto s1 = Renderer::getShader(FBO_PASS);
    auto s2 = Renderer::getShader(FBO_BLUR);
    auto s3 = Renderer::getShader(FBO_HDR);
    camera->fbo->unbind();

    GLuint buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

    camera->fbo->bindTexture(0, 3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    brightPass->bind(1, &buffers[0], false);
    s1->use();
    camera->fbo->draw();
    CHECK_ERROR();

    brightPass->unbind();
    s2->use();
    brightPass->bindTexture(0, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (int i = 0; i < 5; i++)
    {
        (*s2)["level"] = (float)(i + 1);
        (*s2)["width"] = (float)(Renderer::getWindowWidth() / pow(2, i + 1));
        (*s2)["height"] = (float)(Renderer::getWindowHeight() / pow(2, i + 1));
        brightPass->bindTexture(0, 0);
        blurBuffers[i]->bind(1, &buffers[0], false);
        (*s2)["direction"] = glm::vec2(1, 0);
        camera->fbo->draw();

        (*s2)["level"] = 0.f;
        blurBuffers[i]->bindTexture(0, 0);
        blurBuffers[i]->bind(1, &buffers[1], false);
        (*s2)["direction"] = glm::vec2(0, 1);
        camera->fbo->draw();
    }

    blurBuffers[4]->unbind();
    s3->use();
    CHECK_ERROR();

    camera->fbo->bindTexture(0, 3);
    blurBuffers[0]->bindTexture(1, 1);
    blurBuffers[1]->bindTexture(2, 1);
    blurBuffers[2]->bindTexture(3, 1);
    blurBuffers[3]->bindTexture(4, 1);
    blurBuffers[4]->bindTexture(5, 1);
    (*s3)["inputTex"] = 0;
    (*s3)["addTex1"] = 1;
    (*s3)["addTex2"] = 2;
    (*s3)["addTex3"] = 3;
    (*s3)["addTex4"] = 4;
    (*s3)["addTex5"] = 5;
    camera->fbo->draw();
    CHECK_ERROR();
    glfwSwapBuffers(mainWindow);
}
