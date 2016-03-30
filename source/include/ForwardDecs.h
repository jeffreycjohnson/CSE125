#ifndef INCLUDE_FORWARD_DECS_H
#define INCLUDE_FORWARD_DECS_H

#include <algorithm>
#include <string>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <glew.h>

class Animation;
class BoxCollider;
class Camera;
class Component;
class FileWatcher;
class Framebuffer;
class GameObject;
class GPUEmitter;
class Input;
class Light;
class Material;
class MathFunc;
class Mesh;
class Renderer;
class RenderPass;
class Shader;
class Skybox;
class Texture;
class ThreadPool;
class Timer;
class Transform;

#ifdef _MSC_VER
#ifndef __func__
#define __func__ __FUNCSIG__
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

#ifndef CHECK_ERROR
#define CHECK_ERROR() \
do { \
    GLenum error; \
    while ((error = glGetError()) != GL_NO_ERROR) \
    { \
        _log(__FILE__, __LINE__, __func__, \
                std::string("OpenGL Error: ") + (char *)gluErrorString(error)); \
    } \
} while((void)0,0)
#endif

#endif