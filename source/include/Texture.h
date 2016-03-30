#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "ForwardDecs.h"

class Texture
{
	public:
		unsigned int textureHandle;

        explicit Texture(std::string filename, bool srgb = true, GLenum wrap = GL_REPEAT);
		explicit Texture(GLuint handle);
        explicit Texture(glm::vec4 color);
		~Texture();

		void bindTexture(int slot) const;


};

#endif