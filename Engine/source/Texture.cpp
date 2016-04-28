#include "Texture.h"
#include "FileWatcher.h"
#include <SOIL2.h>
#include <unordered_map>

std::unordered_map<std::string, GLuint> textures;

Texture::Texture(const std::string& filename, bool srgb, GLenum wrap) : srgb(srgb) {
    if(textures.count(filename))
    {
        textureHandle = textures[filename];
        return;
    }

    loadFromFile(filename);

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    GLfloat anisoAmt = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoAmt);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoAmt);

	glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_ERROR();

    textures[filename] = textureHandle;
    autoreload = true;
    watcher = std::make_unique<FileWatcher>(filename, 60);
}

void Texture::loadFromFile(const std::string& file, bool reuseHandle)
{

    auto flags = srgb ? SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_SRGB_COLOR_SPACE : 0;
    auto handle = reuseHandle ? textureHandle : SOIL_CREATE_NEW_ID;
    textureHandle = SOIL_load_OGL_texture(file.c_str(), SOIL_LOAD_AUTO, handle, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | flags);
    if (textureHandle == 0) {
        LOG(file + ": Error during loading -> " + SOIL_last_result());
        throw;
    }
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


Texture::Texture(unsigned char buf[], size_t width, size_t height, GLenum format, bool srgb, GLenum wrap)
{
	glGenTextures(1, &textureHandle);

	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
		GL_UNSIGNED_BYTE, buf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_ERROR();
}

Texture::~Texture()
{
    // We cache texture handles now
    //glDeleteTextures(1, &textureHandle);
}

void Texture::bindTexture(int slot) {
    if (autoreload && watcher->changed()) loadFromFile(watcher->file, true);
	int textureSlot = GL_TEXTURE0 + slot;
	glActiveTexture(textureSlot);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
    CHECK_ERROR();
}