#include "Texture.h"
#include <SOIL/SOIL.h>
#include <unordered_map>

std::unordered_map<std::string, GLuint> textures;

Texture::Texture(std::string filename, bool srgb, GLenum wrap) {
    if(textures.count(filename))
    {
        textureHandle = textures[filename];
        return;
    }
    glGenTextures(1, &textureHandle);

    glBindTexture(GL_TEXTURE_2D, textureHandle);
	int width = -1;
	int height = -1;
    unsigned char * image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    if (width == -1) {
        LOG(filename + ": Error during loading -> " + SOIL_last_result());
        throw;
    }

    unsigned char * buffer = static_cast<unsigned char *>(malloc(width * height * 4));
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int i = 0; i < 4; i++)
            {
                buffer[(x + (height - y - 1) * width) * 4 + i] = image[(x + y * width) * 4 + i];
            }
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, buffer);
    free(buffer);
    SOIL_free_image_data(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    GLfloat anisoAmt = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoAmt);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoAmt);

	glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_ERROR();

    textures[filename] = textureHandle;
}

Texture::Texture(GLuint handle) : textureHandle(handle) {
}

Texture::Texture(glm::vec4 color)
{
    auto name = std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b) + std::to_string(color.a);
    if (textures.count(name))
    {
        textureHandle = textures[name];
        return;
    }
    glGenTextures(1, &textureHandle);

    char buffer[4] = { static_cast<char>(color.r * 255), static_cast<char>(color.g * 255), static_cast<char>(color.b * 255), static_cast<char>(color.a * 255) };

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, buffer);

    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_ERROR();

    textures[name] = textureHandle;
}

Texture::~Texture()
{
    // We cache texture handles now
    //glDeleteTextures(1, &textureHandle);
}

void Texture::bindTexture(int slot) const {
	int textureSlot = GL_TEXTURE0 + slot;
	glActiveTexture(textureSlot);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
    CHECK_ERROR();
}