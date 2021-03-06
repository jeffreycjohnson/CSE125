#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "ForwardDecs.h"
#include "Shader.h"
#include "NetworkManager.h"
#include <glfw3.h>  // I'm so sorry, Jeff
#include <vector>
#include <list>

#define NEAR_DEPTH 0.2f
#define FAR_DEPTH 1500.f

#define SHADER_COUNT 21
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
#define SSAO_SHADER 17
#define SSAO_BLUR 18
#define SHADOW_CUBE_SHADER 19
#define UI_SHADER 20

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

		static bool shutdown;

		// Configurable
		static bool enableShadows;
		static Crosshair* crosshair;

		static Shader* currentShader;
        static Camera* mainCamera;
		static Camera* currentCamera;
        static std::list<Camera*> cameras;
		static GPUData gpuData;
        static bool drawDebug;

        static glm::mat4 perspective, view;

		static int shaderForwardLightList[2];

		static void init(int w, int h);
		static void loop(NetworkState caller);

		static void extractObjects();
		static void applyPerFrameData(Camera* camera);
		static void updatePerspective(const glm::mat4& perspectiveMatrix);
		static void setIrradiance(glm::mat4(&irradianceMatrix)[3]);
		static void setEnvironment(int slot, float mipmapLevels);

		static void setCurrentShader(Shader*);
		static Shader& getCurrentShader();
		static Shader& getShader(int shaderId);
		static void switchShader(int shaderId);

		static void setModelMatrix(const glm::mat4& transform);

		static int getWindowWidth() { return windowWidth; }
		static int getWindowHeight() { return windowHeight; }

		static void resize(int width, int height);
		static void focus(GLFWwindow* window, int focused);

		// 2D Drawing (UI Pass)
		// Draws a sprite centered at the given position (in pixels)
		static void drawSprite(glm::vec2 centerPos, glm::vec2 scale, const glm::vec4& color, Texture* image);
		static void drawSpriteStretched(glm::vec2 topLeft, glm::vec2 botRight, const glm::vec4& color, Texture* image);
		static void drawSplash(Texture* image, bool stretch = true);

		// 3D Drawing (Debug Pass)
        static void drawSphere(glm::vec3 pos, float radius, const glm::vec4& color, Transform* transform = nullptr);
        static void drawBox(glm::vec3 pos, const glm::vec3& scale, const glm::vec4& color, Transform* transform = nullptr);
		static void drawArrow(glm::vec3 pos, glm::vec3 dir, const glm::vec4& color, Transform* transform = nullptr);
		static void drawCapsule(glm::vec3 pointA, glm::vec3 pointB, float radius, const glm::vec4& color);

private:
    static int windowWidth, windowHeight;
    Renderer() = delete;
};

#endif