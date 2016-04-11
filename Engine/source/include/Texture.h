#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "ForwardDecs.h"

class Texture
{
    bool autoreload = false;
    FileWatcher * watcher;
    bool srgb = false;

    void loadFromFile(const std::string& file, bool reuseHandle = false);

	public:
		unsigned int textureHandle;

        explicit Texture(const std::string& filename, bool srgb = true, GLenum wrap = GL_REPEAT);
		explicit Texture(GLuint handle);
        explicit Texture(glm::vec4 color);
		~Texture();

		void bindTexture(int slot);
};

#endif