#ifndef INCLUDE_FORWARD_DECS_H
#define INCLUDE_FORWARD_DECS_H

#include <algorithm>
#include <string>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <glew.h>

// Classes
class Animation;
class BoxCollider;
class Camera;
class CapsuleCollider;
class Component;
class Collider;
class CollisionInfo;
class ConfigFile;
class Crosshair;
class FileWatcher;
class Framebuffer;
class GameObject;
class GPUEmitter;
class Input;
class Light;
class Material;
class MathFunc;
class Mesh;
class Octree;
class OctreeNode;
class Plane;
class Ray;
class RayHitInfo;
class Renderer;
class RenderPass;
class Shader;
class Skybox;
class SphereCollider;
class Texture;
class ThreadPool;
class Timer;
class Transform;

// Enum types
enum ColliderType;

#ifdef _MSC_VER
#ifndef __func__
#define __func__ __FUNCSIG__
#endif
#if _MSC_VER < 1900
#define OLD_VS
#define thread_local __declspec( thread )
#endif
#endif

void _log(const char * file, int line, const char * func, const std::string& s);
template<typename T>
void _logHelper(const char * file, int line, const char * func, T val)
{
    _log(file, line, func, std::to_string(val));
}
template<>
void _logHelper<std::string>(const char * file, int line, const char * func, std::string val);
template<>
void _logHelper<const char *>(const char * file, int line, const char * func, const char * val);
template<>
void _logHelper<glm::vec3>(const char * file, int line, const char * func, glm::vec3 val);
template<>
void _logHelper<glm::vec4>(const char * file, int line, const char * func, glm::vec4 val);
template<>
void _logHelper<glm::mat3>(const char * file, int line, const char * func, glm::mat3 val);
template<>
void _logHelper<glm::mat4>(const char * file, int line, const char * func, glm::mat4 val);

#ifndef LOG
#define LOG(X) _logHelper(__FILE__, __LINE__, __func__, X)
#endif

void _fatal(const char * file, int line, const char * func, const char * s);

#ifndef FATAL
#define FATAL(X) _fatal(__FILE__, __LINE__, __func__, X)
#endif

void _assert(const char * file, int line, const char * func, bool expr, const char * s);

#ifndef ASSERT
#define ASSERT(EXPR, X) _assert(__FILE__, __LINE__, __func__, EXPR, X);
#endif

bool _checkError(const char * file, int line, const char * func);
#ifndef CHECK_ERROR
#define CHECK_ERROR() _checkError(__FILE__, __LINE__, __func__)
#endif

#endif