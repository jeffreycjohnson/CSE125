#include "Material.h"
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