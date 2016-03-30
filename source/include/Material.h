#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "ForwardDecs.h"
#include "Texture.h"
#include "Shader.h"
#include <map>

class Material
{
private:
    class UniformType
    {
    public:
        virtual void set(Shader::Uniform) = 0;
        virtual ~UniformType() = default;
    };
    template<typename T>
    class UniformTypeInner : public UniformType
    {
    public:
        T value;
        explicit UniformTypeInner(T value) : value(value) {}

        void set(Shader::Uniform u) override
        {
            u = value;
        };
    };
    std::map<std::string, UniformType*> uniforms;
    std::map<std::string, const Texture*> textures;

    class UniformSetter
    {
    public:
        const std::string& name;
        Material* mat;

        UniformSetter(Material*, const std::string&);
        template<typename T>
        void operator=(T value) {
            if (mat->uniforms.count(name)) delete mat->uniforms[name];
            mat->uniforms[name] = new UniformTypeInner<T>(value);
        }
    };

public:
    Shader * shader;
    bool transparent;

    Material(Shader *, bool transparent = true);
    ~Material();
    UniformSetter operator[](const std::string& name);
    void bind();
};

template<>
void Material::UniformSetter::operator=<Texture*>(Texture* value);
template<>
void Material::UniformSetter::operator=<const Texture*>(const Texture* value);

#endif