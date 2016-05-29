#include "Renderer.h"
#include "Crosshair.h"
#include "Mesh.h"
#include "Camera.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>
#include "Skybox.h"
#include "Timer.h"
#include "Light.h"
#include "Texture.h"
#include "GameObject.h"
#include "ThreadPool.h"
#include "RenderPass.h"
#include "NetworkManager.h"


#define MODEL_MATRIX "uM_Matrix"
#define VIEW_MATRIX "uV_Matrix"

bool Renderer::shutdown = false;
int Renderer::windowWidth = 0;
int Renderer::windowHeight = 0;
bool Renderer::drawDebug = false;
bool Renderer::enableShadows = true;
Crosshair* Renderer::crosshair = nullptr;

glm::mat4 Renderer::view, Renderer::perspective;

Shader* Renderer::currentShader;
Shader* shaderList[SHADER_COUNT];
int Renderer::shaderForwardLightList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM };
int shaderViewList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM, EMITTER_SHADER, EMITTER_BURST_SHADER,
    PARTICLE_TRAIL_SHADER, DEFERRED_PBR_SHADER, DEFERRED_PBR_SHADER_ANIM, DEFERRED_SHADER_LIGHTING, SKYBOX_SHADER,
    SHADOW_SHADER, SHADOW_SHADER_ANIM, DEBUG_SHADER, FORWARD_UNLIT, FORWARD_EMISSIVE, SSAO_SHADER };
int shaderCameraPosList[] = { FORWARD_PBR_SHADER, FORWARD_PBR_SHADER_ANIM, DEFERRED_SHADER_LIGHTING, DEFERRED_PBR_SHADER, DEFERRED_PBR_SHADER_ANIM };
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

    shaderList[SHADOW_CUBE_SHADER] = new Shader(
        "shaders/cube_map.vert", "shaders/cube_map.geom", "shaders/linear_depth.frag"
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

	shaderList[SSAO_BLUR] = new Shader(
		"shaders/fbo.vert", "shaders/ssao_blur.frag"
		);

	shaderList[UI_SHADER] = new Shader(
		"shaders/ui.vert", "shaders/ui.frag"
	);

    mainCamera = new Camera(windowWidth, windowHeight, false);
    mainCamera->passes.push_back(std::make_unique<GBufferPass>());
	mainCamera->passes.push_back(std::make_unique<LightingPass>());
	mainCamera->passes.push_back(std::make_unique<SSAOPass>());
    mainCamera->passes.push_back(std::make_unique<SkyboxPass>(nullptr));
    mainCamera->passes.push_back(std::make_unique<ForwardPass>());
    mainCamera->passes.push_back(std::make_unique<ParticlePass>());
    mainCamera->passes.push_back(std::make_unique<DebugPass>());
    mainCamera->passes.push_back(std::make_unique<UIPass>());
	mainCamera->passes.push_back(std::make_unique<BloomPass>());

    resize(windowWidth, windowHeight);
}

void Renderer::loop(NetworkState caller) {
	if (caller == NetworkState::SERVER_MODE && !drawDebug) {
		std::function<void()> updateScene = std::bind(GameObject::UpdateScene, caller);
		auto job = workerPool->createJob(updateScene)->queue();
		workerPool->wait(job);
		return;
	}
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
			std::function<void()> updateScene = std::bind(GameObject::UpdateScene, caller);
            auto job = workerPool->createJob(updateScene)->queue();
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

void Renderer::focus(GLFWwindow* window, int focused) {
	// "I thought this might do something to help fix the brown-screen-of-death"  -- Dexter
	if (focused == GL_TRUE) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		Renderer::resize(width, height);
	}
}

static void drawWireframe(const std::string& name, glm::vec3 pos, glm::vec3 scale, const glm::vec4& color, glm::mat4 transform)
{
    Renderer::switchShader(DEBUG_SHADER);
    (*Renderer::currentShader)["uColor"] = color;
    (*Renderer::currentShader)["uPosition"] = glm::vec4(pos, 1);
    (*Renderer::currentShader)["uScale"] = glm::vec4(scale, 1);
    Renderer::setModelMatrix(transform);

    MeshData& currentEntry = Mesh::meshMap.at(name);
    if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
        glBindVertexArray(currentEntry.vaoHandle);
        Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
    }

    glDrawElements(GL_LINES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
}

void Renderer::drawSprite(glm::vec2 centerPos, glm::vec2 scale, const glm::vec4& color, Texture* image) {
	// TODO: There is probably some sort of optimization to be done here or in UIPass (ForwardPass.cpp)

	auto& shader = Renderer::getShader(UI_SHADER);
	auto& currentEntry = Mesh::meshMap["assets/Primatives.obj/Plane"];

	if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
		glBindVertexArray(currentEntry.vaoHandle);
		Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
	}

	// "pos" is in pixels.
	float t1 = centerPos.x / Renderer::getWindowWidth();
	float t2 = centerPos.y / Renderer::getWindowHeight();
	float screenSpaceX = (-1.0 * (1.0 - t1)) + t1;
	float screenSpaceY = (1.0 * (1.0 - t2)) - t2; // invert Y axis

	auto pixelPerfectScale = glm::vec2((float)image->getWidth() / Renderer::getWindowWidth(), (float)-image->getHeight() / Renderer::getWindowHeight());

	shader.use();
	shader["uColor"] = glm::vec4(1);
	image->bindTexture(0);
	shader["tex"] = 0;
	shader["scale"] = pixelPerfectScale * scale;
	shader["translation"] = glm::vec4(glm::vec2(screenSpaceX, screenSpaceY), 0, 0);
	glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
	CHECK_ERROR();
}

void Renderer::drawSpriteStretched(glm::vec2 topLeft, glm::vec2 botRight, const glm::vec4& color, Texture* image) {
	auto& shader = Renderer::getShader(UI_SHADER);
	auto& currentEntry = Mesh::meshMap["assets/Primatives.obj/Plane"];

	if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
		glBindVertexArray(currentEntry.vaoHandle);
		Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
	}

	glm::vec2 centerPos = (topLeft + botRight);
	centerPos /= 2;

	// I don't want to deal with this
	ASSERT(botRight.x >= topLeft.x, "Bottom right x coordinate less than top left x coordinate.");
	ASSERT(botRight.y >= topLeft.y, "Bottom right y coordinate less than top left y coordinate.");

	float desiredW = botRight.x - topLeft.x;
	float desiredH = botRight.y - topLeft.y;
	float imageW = image->getWidth();
	float imageH = image->getHeight();

	// "pos" is in pixels.
	float t1 = centerPos.x / Renderer::getWindowWidth();
	float t2 = centerPos.y / Renderer::getWindowHeight();
	float screenSpaceX = (-1.0 * (1.0 - t1)) + t1;
	float screenSpaceY = (1.0 * (1.0 - t2)) - t2; // invert Y axis

	auto pixelPerfectScale = glm::vec2( imageW / (float)Renderer::getWindowWidth(), -imageH / (float)Renderer::getWindowHeight());
	auto scale = glm::vec2(desiredW / imageW, desiredH / imageH);

	shader.use();
	shader["uColor"] = glm::vec4(1);
	image->bindTexture(0);
	shader["tex"] = 0;
	shader["scale"] = pixelPerfectScale * scale;
	shader["translation"] = glm::vec4(glm::vec2(screenSpaceX, screenSpaceY), 0, 0);
	glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
	CHECK_ERROR();
}


void Renderer::drawSplash(Texture * image, bool stretch)
{
	// Draws a splash screen image in the center of the screen. Set stretch to true to gurantee it fills the screen.
	// Otherwise, it will be pixel perfect.
	auto& shader = Renderer::getShader(UI_SHADER);
	auto& currentEntry = Mesh::meshMap["assets/Primatives.obj/Plane"];

	if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
		glBindVertexArray(currentEntry.vaoHandle);
		Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
	}

	auto pixelPerfectScale = glm::vec2((float)image->getWidth() / Renderer::getWindowWidth(), (float)-image->getHeight() / Renderer::getWindowHeight());

	shader.use();
	shader["uColor"] = glm::vec4(1);
	image->bindTexture(0);
	shader["tex"] = 0;
	shader["scale"] = stretch ? glm::vec2(1,-1) : pixelPerfectScale;
	shader["translation"] = glm::vec4(0);
	glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
	CHECK_ERROR();
}

void Renderer::drawSphere(glm::vec3 pos, float radius, const glm::vec4& color, Transform* transform)
{
    drawWireframe("assets/Primatives.obj/Sphere_Outline", pos, glm::vec3(radius), color, transform ? transform->getTransformMatrix() : glm::mat4());
}

void Renderer::drawBox(glm::vec3 pos, const glm::vec3& scale, const glm::vec4& color, Transform* transform)
{
    drawWireframe("assets/Primatives.obj/Box_Outline", pos, scale, color, transform ? transform->getTransformMatrix() : glm::mat4());
}

void Renderer::drawArrow(glm::vec3 pos, glm::vec3 dir, const glm::vec4& color, Transform* transform)
{
	auto rot = glm::rotation(glm::vec3(0,1,0), glm::normalize(dir));
	auto mat = glm::translate(glm::mat4_cast(rot), pos);
	if(transform) mat = mat * transform->getTransformMatrix();
	drawWireframe("assets/Primatives.obj/Arrow_Outline", glm::vec3(0), glm::vec3(glm::length(dir) / 10.f), color, mat);
}

void Renderer::drawCapsule(glm::vec3 pointA, glm::vec3 pointB, float radius, const glm::vec4& color) {
	// TODO: Actually draw a capsule later, for now I will just use two spheres
	drawSphere(pointA, radius, color);
	drawSphere(pointB, radius, color);
}