#include "Shader.h"
#include <fstream>
#include <gtc/type_ptr.hpp>
#include "FileWatcher.h"
#include "Renderer.h"

Shader::Uniform::Uniform(GLint program, GLint location)
    : program(program), location(location)
{
}

void Shader::Uniform::operator=(bool val)
{
    glProgramUniform1i(program, location, val);
}

void Shader::Uniform::operator=(float val)
{
    glProgramUniform1f(program, location, val);
}

void Shader::Uniform::operator=(int val)
{
    glProgramUniform1i(program, location, val);
}

void Shader::Uniform::operator=(unsigned int val)
{
    glProgramUniform1ui(program, location, val);
}

void Shader::Uniform::operator=(const glm::vec2& val)
{
    glProgramUniform2fv(program, location, 1, glm::value_ptr(val));
}

void Shader::Uniform::operator=(const glm::vec3& val)
{
    glProgramUniform3fv(program, location, 1, glm::value_ptr(val));
}

void Shader::Uniform::operator=(const glm::vec4& val)
{
    glProgramUniform4fv(program, location, 1, glm::value_ptr(val));
}

void Shader::Uniform::operator=(const glm::mat2& val)
{
    glProgramUniformMatrix2fv(program, location, 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::Uniform::operator=(const glm::mat3& val)
{
    glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::Uniform::operator=(const glm::mat4& val)
{
    glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, glm::value_ptr(val));
}

static std::vector<std::string> tokenize(std::string s)
{
    std::vector<std::string> ret;
    std::string current;
    for(auto c : s)
    {
        if (c != ' ' && c != '\t' && c != '\n' && c !='\r') current += c;
        else
        {
            if(current.length() > 0) ret.push_back(current);
            current = "";
        }
    }
    if (current.length() > 0) ret.push_back(current);
    return ret;
}

static std::string readShader(const std::string& name)
{
    std::ifstream data(name, std::ios::binary);
    if(data.fail())
    {
        LOG(name + " unable to be opened.");
        return "";
    }
    std::string line, ret;
    while(data.good())
    {
        std::getline(data, line);
        auto tokens = tokenize(line);
        if (tokens.size() == 0) continue;
        if (tokens[0] == "#include")
        {
            ret += readShader(tokens[1]);
        }
        else ret += line + '\n';
    }
    data.close();
    return ret;
}

static bool compile(const std::string& file, GLuint shader)
{
    std::string data = readShader(file);
    char const* buffer = data.c_str();

    glShaderSource(shader, 1, &buffer, nullptr);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char errbuf[512];
        glGetShaderInfoLog(shader, 512, nullptr, errbuf);
        LOG(file);
        LOG((static_cast<const char *>(errbuf)));
        return true;
    }
    return false;
}

Shader::Shader(const std::string& vertex, const std::string& fragment)
{
    vertWatcher = std::make_unique<FileWatcher>(vertex, 60);
    fragWatcher = std::make_unique<FileWatcher>(fragment, 60);
    reload();
}

Shader::Shader(const std::string& vertex, const std::string& geom, const std::string& fragment)
{
    vertWatcher = std::make_unique<FileWatcher>(vertex, 60);
    fragWatcher = std::make_unique<FileWatcher>(fragment, 60);
    geomWatcher = std::make_unique<FileWatcher>(geom, 60);
    reload();
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

//TODO should we pre-extract these into a map?
Shader::Uniform Shader::operator[](const std::string& name)
{
    auto loc = glGetUniformLocation(id, name.c_str());
    if(id == -1 || loc == -1)
    {
        CHECK_ERROR();
        //LOG(name + "not defined correctly.");
    }
    return Uniform(id, loc);
}

void Shader::use()
{
    if (this == Renderer::currentShader) return;
    if(autoReload && (vertWatcher->changed() || fragWatcher->changed() || (geomWatcher && geomWatcher->changed()))) reload();
    glUseProgram(id);
	Renderer::setCurrentShader(this);
}

void Shader::reload()
{
    if(id != -1) glDeleteProgram(id);

    id = glCreateProgram();
    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    auto geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    CHECK_ERROR();

    if (compile(vertWatcher->file, vertexShader)) FATAL(vertWatcher->file.c_str());
    glAttachShader(id, vertexShader);
    if (compile(fragWatcher->file, fragmentShader)) FATAL(fragWatcher->file.c_str());
    glAttachShader(id, fragmentShader);
    if(geomWatcher)
    {
        if (compile(geomWatcher->file, geometryShader)) FATAL(geomWatcher->file.c_str());
        glAttachShader(id, geometryShader);
    }

    glLinkProgram(id);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);
    CHECK_ERROR();

    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        char errbuf[512];
        glGetProgramInfoLog(id, 512, nullptr, errbuf);
        FATAL(static_cast<const char *>(errbuf));
    }
    CHECK_ERROR();
}