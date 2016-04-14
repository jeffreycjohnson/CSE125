#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "ForwardDecs.h"

class Texture
{
    bool autoreload = false;
    std::unique_ptr<FileWatcher> watcher;
    bool srgb = false;

    void loadFromFile(const std::string& file, bool reuseHandle = false);

	public:
		unsigned int textureHandle;

        explicit Texture(const std::string& filename, bool srgb = true, GLenum wrap = GL_REPEAT);
		explicit Texture(GLuint handle);
        explicit Texture(glm::vec4 color);
		Texture(unsigned char buf[], size_t width, size_t height, GLenum format, bool srgb = false, GLenum wrap = GL_REPEAT);
		~Texture();

		void bindTexture(int slot);
};

#endif