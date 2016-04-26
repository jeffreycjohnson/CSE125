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
    std::map<std::string, std::unique_ptr<UniformType>> uniforms;
    std::map<std::string, std::unique_ptr<Texture>> textures;
    std::unique_ptr<FileWatcher> watcher;
    bool hasAnimations = false;

    class UniformSetter
    {
    public:
        const std::string& name;
        Material* mat;

        UniformSetter(Material*, const std::string&);
        template<typename T>
        void operator=(T value) {
            mat->uniforms[name] = std::make_unique<UniformTypeInner<T>>(value);
        }
    };

    void loadFromFile(const std::string& file);

public:
    Shader * shader;
    bool transparent;

    explicit Material(Shader *, bool transparent = true);
    explicit Material(const std::string& file, bool hasAnimations=false);
    UniformSetter operator[](const std::string& name);
    void bind();
    const bool autoReload = false;
	std::string getWatcherFileName() const;

};

template<>
void Material::UniformSetter::operator=<std::unique_ptr<Texture>>(std::unique_ptr<Texture> value);

#endif