#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include <gtc/matrix_transform.hpp>
#include "Skybox.h"
#include "Timer.h"
#include "Light.h"
#include "GameObject.h"
#include "ThreadPool.h"
#include "RenderPass.h"


#define MODEL_MATRIX "uM_Matrix"
#define VIEW_MATRIX "uV_Matrix"

bool Renderer::shutdown = false;
int Renderer::windowWidth = 0;
int Renderer::windowHeight = 0;
bool Renderer::drawDebug = false;

glm::mat4 Renderer::view, Renderer::perspective;

Shader* Renderer::currentShader;
Shader* shaderList[SHADER_COUNT];
int Renderer::shaderForwardLightList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM };
int shaderViewList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM, EMITTER_SHADER, EMITTER_BURST_SHADER,
    PARTICLE_TRAIL_SHADER, DEFERRED_PBR_SHADER, DEFERRED_PBR_SHADER_ANIM, DEFERRED_SHADER_LIGHTING, SKYBOX_SHADER,
    SHADOW_SHADER, SHADOW_SHADER_ANIM, DEBUG_SHADER, FORWARD_UNLIT, FORWARD_EMISSIVE };
int shaderCameraPosList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM, DEFERRED_SHADER_LIGHTING };
int shaderEnvironmentList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM, DEFERRED_SHADER_LIGHTING };
int shaderPerspectiveList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM, SKYBOX_SHADER, EMITTER_SHADER,
    EMITTER_BURST_SHADER, PARTICLE_TRAIL_SHADER, DEFERRED_PBR_SHADER, DEFERRED_PBR_SHADER_ANIM, DEFERRED_SHADER_LIGHTING,
    DEBUG_SHADER, FORWARD_UNLIT, FORWARD_EMISSIVE, SSAO_SHADER };

Camera* Renderer::mainCamera;
Camera* Renderer::currentCamera;
std::list<Camera*> Renderer::cameras;

GPUData Renderer::gpuData;

Renderer::RenderBuffer Renderer::renderBuffer;

Skybox* skybox;

void Renderer::init(int window_width, int window_height) {
    windowWidth = window_width;
    windowHeight = window_height;

	gpuData.vaoHandle = -1;

	//Set Enables
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0, 0, 0, 1);
	glDepthFunc(GL_LEQUAL); //needed for skybox to overwrite blank z-buffer values

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	shaderList[FORWARD_PBR_SHADER_ANIM] = new Shader(
		"shaders/forward_pbr_skeletal.vert", "shaders/forward_pbr.frag"
		);


	shaderList[FORWARD_PBR_SHADER] = new Shader(
		"shaders/forward_pbr.vert", "shaders/forward_pbr.frag"
		);

    shaderList[DEFERRED_PBR_SHADER_ANIM] = new Shader(
        "shaders/forward_pbr_skeletal.vert", "shaders/deferred_gbuffer.frag"
        );


    shaderList[DEFERRED_PBR_SHADER] = new Shader(
        "shaders/forward_pbr.vert", "shaders/deferred_gbuffer.frag"
        );

    shaderList[DEFERRED_SHADER_LIGHTING] = new Shader(
        "shaders/deferred_lighting.vert", "shaders/deferred_lighting.frag"
        );

	shaderList[SKYBOX_SHADER] = new Shader(
		"shaders/skybox.vert", "shaders/skybox.frag"
		);

	shaderList[FBO_HDR] = new Shader(
		"shaders/fbo.vert", "shaders/fbo_hdr.frag"
		);

	shaderList[EMITTER_SHADER] = new Shader(
		"shaders/gpu_particle.vert", "shaders/gpu_particle.frag"
		);

	shaderList[EMITTER_BURST_SHADER] = new Shader(
		"shaders/gpu_particle_burst.vert", "shaders/gpu_particle.frag"
		);

	shaderList[PARTICLE_TRAIL_SHADER] = new Shader(
		"shaders/particle_trail.vert", "shaders/particle_trail.frag"
		);

    shaderList[SHADOW_SHADER] = new Shader(
        "shaders/forward_pbr.vert", "shaders/shadow.frag"
        );

    shaderList[SHADOW_SHADER_ANIM] = new Shader(
        "shaders/forward_pbr_skeletal.vert", "shaders/shadow.frag"
        );

    (*shaderList[SHADOW_SHADER])["uP_Matrix"] = DirectionalLight::shadowMatrix;
    (*shaderList[SHADOW_SHADER_ANIM])["uP_Matrix"] = DirectionalLight::shadowMatrix;

	shaderList[DEBUG_SHADER] = new Shader(
		"shaders/simple.vert", "shaders/simple.frag"
		);

    shaderList[FORWARD_UNLIT] = new Shader(
        "shaders/forward_pbr.vert", "shaders/forward_unlit.frag"
        );
    shaderList[FORWARD_EMISSIVE] = new Shader(
        "shaders/forward_pbr.vert", "shaders/emissive.frag"
        );

	shaderList[FBO_BLUR] = new Shader(
		"shaders/fbo.vert", "shaders/fbo_blur.frag"
		);

	shaderList[FBO_PASS] = new Shader(
		"shaders/fbo.vert", "shaders/fbo_pass.frag"
		);

	shaderList[SSAO_SHADER] = new Shader(
		"shaders/fbo.vert", "shaders/ssao.frag"
		);

    mainCamera = new Camera(windowWidth, windowHeight, false);
    mainCamera->passes.push_back(std::make_unique<GBufferPass>());
	mainCamera->passes.push_back(std::make_unique<LightingPass>());
	mainCamera->passes.push_back(std::make_unique<SSAOPass>());
    mainCamera->passes.push_back(std::make_unique<SkyboxPass>(nullptr));
    mainCamera->passes.push_back(std::make_unique<ForwardPass>());
    mainCamera->passes.push_back(std::make_unique<ParticlePass>());
    mainCamera->passes.push_back(std::make_unique<DebugPass>());
    mainCamera->passes.push_back(std::make_unique<BloomPass>());

    resize(windowWidth, windowHeight);
}

void Renderer::loop() {
    extractObjects();

    for(auto camera : cameras)
    {
        camera->update((float)Timer::deltaTime());
        if (camera == mainCamera) continue;
        currentCamera = camera;
        applyPerFrameData(camera);
        for(auto& pass : camera->passes)
        {
            pass->render(camera);
        }
    }
    currentCamera = mainCamera;
    applyPerFrameData(mainCamera);
    int i = 0;
    for (auto& pass : mainCamera->passes)
    {
        if (i++ == mainCamera->passes.size() - 1) {
            auto job = workerPool->createJob(GameObject::UpdateScene)->queue();
            pass->render(mainCamera);
            workerPool->wait(job);
        }
        else pass->render(mainCamera);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::extractObjects() {
    renderBuffer.deferred.clear();
    renderBuffer.forward.clear();
    renderBuffer.particle.clear();
    renderBuffer.light.clear();
	GameObject::SceneRoot.extract();
}

void Renderer::applyPerFrameData(Camera* camera) {
    view = camera->getCameraMatrix();
	for (int shaderId : shaderViewList) {
		getShader(shaderId)[VIEW_MATRIX] = view;
	}
	for (int shaderId : shaderCameraPosList) {
		getShader(shaderId)["cameraPos"] = camera->gameObject->transform.getWorldPosition();
	}
}

void Renderer::updatePerspective(const glm::mat4& perspectiveMatrix) {
    perspective = perspectiveMatrix;
	for (int shaderId : shaderPerspectiveList) {
		getShader(shaderId)["uP_Matrix"] = perspective;
	}
}

void Renderer::setEnvironment(int slot, float mipmapLevels) {
	for (int shaderId : shaderEnvironmentList) {
		getShader(shaderId)["environment"] = slot;
		getShader(shaderId)["environment_mipmap"] = mipmapLevels;
	}
}

void Renderer::setIrradiance(glm::mat4 (&irradianceMatrix)[3]) {
    for (int shaderId : shaderEnvironmentList) {
        getShader(shaderId)["irradiance[0]"] = irradianceMatrix[0];
        getShader(shaderId)["irradiance[1]"] = irradianceMatrix[1];
        getShader(shaderId)["irradiance[2]"] = irradianceMatrix[2];
    }
}


Shader& Renderer::getCurrentShader() {
	return *currentShader;
}

Shader& Renderer::getShader(int shaderId) {
	return *shaderList[shaderId];
}

void Renderer::setCurrentShader(Shader* shader) {
	currentShader = shader;
}

void Renderer::switchShader(int shaderId) {
	auto shader = shaderList[shaderId];
    shader->use();
    currentShader = shader;
}

void Renderer::setModelMatrix(const glm::mat4& transform) {
	(*currentShader)[MODEL_MATRIX] = transform;
}

void Renderer::resize(int width, int height) {
	glViewport(0, 0, width, height);

	windowWidth = width;
	windowHeight = height;

	perspective = glm::perspective(mainCamera->getFOV(), width / (float)height, NEAR_DEPTH, FAR_DEPTH);
	updatePerspective(perspective);
}

void Renderer::drawSphere(glm::vec3 pos, float radius, const glm::vec4& color, Transform* transform)
{
    Mesh m("Sphere_Outline");
    switchShader(DEBUG_SHADER);
    glm::vec4 position(pos, 1);
    if (transform) m.setGameObject(transform->gameObject);
    (*currentShader)["uPosition"] = position;
    (*currentShader)["uScale"] = glm::vec4(radius);
    (*currentShader)["uColor"] = color;
    m.draw();
}

void Renderer::drawBox(glm::vec3 pos, const glm::vec3& scale, const glm::vec4& color, Transform* transform)
{
    Mesh m("Box_Outline");
    switchShader(DEBUG_SHADER);
    glm::vec4 position(pos, 1);
    if (transform) m.setGameObject(transform->gameObject);
    (*currentShader)["uPosition"] = position;
    (*currentShader)["uScale"] = glm::vec4(scale, 1);
    (*currentShader)["uColor"] = color;
    m.draw();
}
