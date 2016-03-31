#include "RenderPass.h"
#include "Skybox.h"

SkyboxPass::SkyboxPass(Skybox* skybox) : skybox(skybox) {
}

void SkyboxPass::render(Camera*) { if (skybox) skybox->draw(); }