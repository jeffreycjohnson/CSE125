#include "Material.h"
#include "Config.h"
#include "Renderer.h"
#include "FileWatcher.h"

Material::UniformSetter::UniformSetter(Material* mat, const std::string& name) : name(name), mat(mat)
{
}

static GLenum parseWrapMode(const std::string& str)
{
    if (str == "Repeat") return GL_REPEAT;
    else return GL_CLAMP_TO_EDGE;
}

static void loadTexture(const ConfigFile& config, Material& mat, const std::string& section, const std::string& name, const std::string& uniform, bool srgb)
{
    auto tmp = config.getColor(section, name);
    mat[uniform] = std::make_unique<Texture>(glm::vec4(tmp, 1));
    auto tex = config.getString(section, name + "Texture");
    auto wrapMode = config.getString(section, name + "TextureWrapping");
    if (tex != "") mat[uniform] = std::make_unique<Texture>(tex, srgb, parseWrapMode(wrapMode));
}

void Material::loadFromFile(const std::string& file)
{
    ConfigFile config(file);

    bool customShader = false;
    transparent = false;
    if (config.hasSection("Shader"))
    {
        auto vertexShader = config.getString("Shader", "VertexShader");
        auto fragmentShader = config.getString("Shader", "FragmentShader");
        if (vertexShader != "" && fragmentShader != "")
        {
            shader = new Shader(vertexShader, fragmentShader);
            customShader = true;
        }

        auto renderPath = config.getString("Shader", "RenderPath");
        if (renderPath != "Deferred") transparent = true;
    }

    if (config.hasSection("Diffuse"))
    {
        loadTexture(config, *this, "Diffuse", "Diffuse", "colorTex", true);
    }
    else
    {
        (*this)["colorTex"] = std::make_unique<Texture>(glm::vec4(1));
    }
    if (config.hasSection("Normal"))
    {
        loadTexture(config, *this, "Normal", "Normal", "normalTex", false);
    }
    else
    {
        (*this)["normalTex"] = std::make_unique<Texture>(glm::vec4(0.5, 0.5, 1, 1));
    }
    if (config.hasSection("Height"))
    {
        loadTexture(config, *this, "Height", "Height", "heightTex", false);
        (*this)["uParallax"] = true;
        (*this)["uDepthScale"] = config.getFloat("Height", "DepthScale");
    }
    else
    {
        (*this)["heightTex"] = std::make_unique<Texture>(glm::vec4(0));
    }
    if (config.hasSection("Roughness"))
    {
        loadTexture(config, *this, "Roughness", "Roughness", "roughnessTex", false);
    }
    else
    {
        (*this)["roughnessTex"] = std::make_unique<Texture>(glm::vec4(0.5));
    }
    if (config.hasSection("Metalness"))
    {
        loadTexture(config, *this, "Metalness", "Metalness", "metalnessTex", false);
    }
    else
    {
        (*this)["metalnessTex"] = std::make_unique<Texture>(glm::vec4(0));
    }

    if (config.hasSection("Emission"))
    {
        transparent = true;
        (*this)["emissionStrength"] = config.getFloat("Emission", "Strength");
        loadTexture(config, *this, "Emission", "Color", "colorTex", true);
        if (!customShader) shader = &Renderer::getShader(FORWARD_EMISSIVE);
    }
    else if (transparent)
    {
        if (hasAnimations) shader = &Renderer::getShader(FORWARD_PBR_SHADER_ANIM);
        else shader = &Renderer::getShader(FORWARD_PBR_SHADER);
    }
    else
    {
        if (hasAnimations) shader = &Renderer::getShader(DEFERRED_PBR_SHADER_ANIM);
        else shader = &Renderer::getShader(DEFERRED_PBR_SHADER);
    }
}

template<>
void Material::UniformSetter::operator=<std::unique_ptr<Texture>>(std::unique_ptr<Texture> value) {
    mat->textures[name] = std::move(value);
}

Material::Material(Shader* shader, bool transparent) : shader(shader), transparent(transparent)
{
}

Material::Material(const std::string& file, bool hasAnimations) : hasAnimations(hasAnimations), autoReload(true)
{
    loadFromFile(file);
    watcher = std::make_unique<FileWatcher>(file, 30);
}

Material::UniformSetter Material::operator[](const std::string& name)
{
    return UniformSetter(this, name);
}

void Material::bind()
{
    if (autoReload && watcher->changed()) loadFromFile(watcher->file);
    shader->use();
    for (auto& uniform : uniforms)
    {
        uniform.second->set((*shader)[uniform.first]);
    }
    int i = 0; //sampler seems to prefer glUniform1i
    for (auto& texture : textures)
    {
        texture.second->bindTexture(i);
        (*shader)[texture.first] = i;
        i++;
    }
}

std::string Material::getWatcherFileName() const
{
	return watcher->file;
}
