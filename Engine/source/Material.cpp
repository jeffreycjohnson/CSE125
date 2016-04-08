#include "Material.h"
#include "Config.h"
#include "Renderer.h"

Material::UniformSetter::UniformSetter(Material* mat, const std::string& name) : name(name), mat(mat)
{
}

template<>
void Material::UniformSetter::operator=<Texture*>(Texture* value) {
    mat->textures[name] = value;
}

template<>
void Material::UniformSetter::operator=<const Texture*>(const Texture* value) {
    mat->textures[name] = value;
}

Material::Material(Shader* shader, bool transparent) : shader(shader), transparent(transparent)
{
}

static GLenum parseWrapMode(const std::string& str)
{
    if (str == "Repeat") return GL_REPEAT;
    else return GL_CLAMP_TO_EDGE;
}

static void loadTexture(const ConfigFile& config, Material& mat, const std::string& section, const std::string& name, const std::string& uniform, bool srgb)
{
    mat[uniform] = config.getColor(section, name);
    auto tex = config.getString(section, name + "Texture");
    auto wrapMode = config.getString(section, name + "TextureWrapping");
    if (tex != "") mat[uniform] = new Texture(tex, srgb, parseWrapMode(wrapMode));
}

Material::Material(const std::string& file, bool hasAnimations)
{
    ConfigFile config(file);

    bool customShader = false;
    if(config.hasSection("Shader"))
    {
        auto vertexShader = config.getString("Shader", "VertexShader");
        auto fragmentShader = config.getString("Shader", "FragmentShader");
        if (vertexShader != "" && fragmentShader != "")
        {
            shader = new Shader(vertexShader, fragmentShader);
            customShader = true;
        }

        transparent = false;
        auto renderPath = config.getString("Shader", "RenderPath");
        if (renderPath != "Deferred") transparent = true;
    }

    if(config.hasSection("Emission"))
    {
        transparent = true;
        (*this)["emissionStrength"] = config.getFloat("Emission", "Strength");
        loadTexture(config, *this, "Emission", "Color", "emissionColor", true);
        if (!customShader) shader = &Renderer::getShader(FORWARD_EMISSIVE);
    }
    else if(transparent)
    {
        if(hasAnimations) shader = &Renderer::getShader(FORWARD_PBR_SHADER_ANIM);
        else shader = &Renderer::getShader(FORWARD_PBR_SHADER);
    }
    else
    {
        if (hasAnimations) shader = &Renderer::getShader(DEFERRED_PBR_SHADER_ANIM);
        else shader = &Renderer::getShader(DEFERRED_PBR_SHADER);
    }

    if (config.hasSection("Diffuse"))
    {
        loadTexture(config, *this, "Diffuse", "Diffuse", "colorTex", true);
    }
    if (config.hasSection("Normal"))
    {
        loadTexture(config, *this, "Normal", "Normal", "normalTex", false);
    }
    if (config.hasSection("Height"))
    {
        loadTexture(config, *this, "Height", "Height", "heightTex", false);
    }
    if (config.hasSection("Roughness"))
    {
        loadTexture(config, *this, "Roughness", "Roughness", "roughnessTex", false);
    }
    if (config.hasSection("Metalness"))
    {
        loadTexture(config, *this, "Metalness", "Metalness", "metalnessTex", false);
    }
}

Material::~Material()
{
    for (auto uniform : uniforms) {
        if(uniform.second) delete uniform.second;
    }
}

Material::UniformSetter Material::operator[](const std::string& name)
{
    return UniformSetter(this, name);
}

void Material::bind()
{
    shader->use();
    for (auto uniform : uniforms)
    {
        uniform.second->set((*shader)[uniform.first]);
    }
    int i = 0; //sampler seems to prefer glUniform1i
    for (auto texture : textures)
    {
        texture.second->bindTexture(i);
        (*shader)[texture.first] = i;
        i++;
    }
}