#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "ForwardDecs.h"
#include "Shader.h"
#include <vector>
#include <list>

#define NEAR_DEPTH 0.2f
#define FAR_DEPTH 1500.f

#define SHADER_COUNT 17
#define FORWARD_PBR_SHADER_ANIM 0
#define FORWARD_PBR_SHADER 1
#define SKYBOX_SHADER 2
#define FBO_HDR 3
#define EMITTER_SHADER 4
#define EMITTER_BURST_SHADER 5
#define DEFERRED_PBR_SHADER_ANIM 6
#define DEFERRED_PBR_SHADER 7
#define DEFERRED_SHADER_LIGHTING 8
#define PARTICLE_TRAIL_SHADER 9
#define SHADOW_SHADER 10
#define SHADOW_SHADER_ANIM 11
#define DEBUG_SHADER 12
#define FORWARD_UNLIT 13
#define FORWARD_EMISSIVE 14
#define FBO_BLUR 15
#define FBO_PASS 16

#define FORWARD_SHADER_LIGHT_MAX 5

struct GPUData {
	GLuint vaoHandle;
	int textureSlot[20];
};

class Renderer
{
	public:
        struct RenderBuffer {
            std::vector<Mesh*> forward;
            std::vector<Mesh*> deferred;
            std::vector<Component*> particle;
            std::vector<Light*> light;
        };
        static struct RenderBuffer renderBuffer;

		static Shader* currentShader;
        static Camera* mainCamera;
		static Camera* currentCamera;
        static std::list<Camera*> cameras;
		static GPUData gpuData;
        static bool drawDebug;

        static glm::mat4 perspective, view;

		static int shaderForwardLightList[2];

		static void init(int w, int h);
		static void loop();

		static void extractObjects();
		static void applyPerFrameData(Camera* camera);
		static void updatePerspective(const glm::mat4& perspectiveMatrix);
		static void setIrradiance(glm::mat4(&irradianceMatrix)[3]);
		static void setEnvironment(int slot, float mipmapLevels);

		static void setCurrentShader(Shader*);
		static Shader& getCurrentShader();
		static Shader* getShader(int shaderId);
		static void switchShader(int shaderId);

		static void setModelMatrix(const glm::mat4& transform);

		static int getWindowWidth() { return windowWidth; }
		static int getWindowHeight() { return windowHeight; }

		static void resize(int width, int height);

        static void drawSphere(glm::vec3 pos, float radius, const glm::vec4& color, Transform* transform = nullptr);
        static void drawBox(glm::vec3 pos, const glm::vec3& scale, const glm::vec4& color, Transform* transform = nullptr);

private:
    static int windowWidth, windowHeight;
    Renderer() = delete;
};

#endif