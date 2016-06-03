#include "ForwardDecs.h"
#include <iostream>
#include <glfw3.h>

template<>
void _logHelper<std::string>(const char * file, int line, const char * func, std::string val)
{
    _log(file, line, func, val);
}

template<>
void _logHelper<const char *>(const char * file, int line, const char * func, const char * val)
{
    _log(file, line, func, val);
}

template<>
void _logHelper<glm::vec3>(const char * file, int line, const char * func, glm::vec3 val)
{
    std::string str = "[" + std::to_string(val.x) + ", " + std::to_string(val.y) +
        ", " + std::to_string(val.z) + "]";
    _log(file, line, func, str);
}

template<>
void _logHelper<glm::vec4>(const char * file, int line, const char * func, glm::vec4 val)
{
    std::string str = "[" + std::to_string(val.x) + ", " + std::to_string(val.y) + ", " +
        std::to_string(val.z) + ", " + std::to_string(val.w) + "]";
    _log(file, line, func, str);
}

template<>
void _logHelper<glm::mat3>(const char * file, int line, const char * func, glm::mat3 val)
{
    std::string str = "[";
    for (int i = 0; i < 3; i++)
    {
        str += "[" + std::to_string(val[i][0]) + ", " + std::to_string(val[i][1]) +
            ", " + std::to_string(val[i][2]) + "]";
        if (i != 2) str += "\n";
    }
    str += "]";
    _log(file, line, func, str);
}

template<>
void _logHelper<glm::mat4>(const char * file, int line, const char * func, glm::mat4 val)
{
    std::string str = "[";
    for (int i = 0; i < 4; i++)
    {
        str += "[" + std::to_string(val[i][0]) + ", " + std::to_string(val[i][1]) +
            ", " + std::to_string(val[i][2]) + ", " + std::to_string(val[i][3]) + "]";
        if (i != 3) str += "\n";
    }
    str += "]";
    _log(file, line, func, str);
}

void _log(const char * file, int line, const char * func, const std::string& s)
{
    std::cerr << s << " in" << std::endl;
    std::cerr << file << " in function " << func << " on line " << line << std::endl;
}

// Fatal

void _fatal(const char * file, int line, const char * func, const char * s) {
	std::cerr << "================================================" << std::endl;
	std::cerr << "  /!\\   FATAL Engine Error!" << std::endl;
	std::cerr << "================================================" << std::endl; 
	std::cerr << "File: " << file << " line " << line << std::endl; 
	std::cerr << "Function: " << func << std::endl;
	std::cerr << "Message: " << s << std::endl;
	glfwTerminate();
	while (true) {};
}

// Assert

void _assert(const char * file, int line, const char * func, bool expr, const char * s) {
	if (!expr) {
		std::cerr << "================================================" << std::endl;
		std::cerr << "  /?\\   Engine Assertion Failed!" << std::endl;
		std::cerr << "================================================" << std::endl;
		std::cerr << "File: " << file << " line " << line << std::endl;
		std::cerr << "Function: " << func << std::endl;
		std::cerr << "Message: " << s << std::endl;
		glfwTerminate();
		while (true) {};
	}
}


bool _checkError(const char * file, int line, const char * func) {
    GLenum error;
    bool hadError = false;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        hadError = true;
        char * str = (char*)gluErrorString(error);
        _log(file, line, func,
            std::string("OpenGL Error: ") + (str ? str : ""));
    }
    return hadError;
}