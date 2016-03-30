#ifndef INCLUDE_FRAMEBUFFER_H
#define INCLUDE_FRAMEBUFFER_H

#include "ForwardDecs.h"
#include "Mesh.h"
#include <vector>

class Framebuffer
{
public:
	GLuint id;
	bool accessibleDepth;
	GLuint *colorTex;
	int numColorTex;
	GLuint depthTex;

	bool hdrEnabled=false;
    std::vector<GLint> colorFormats;

	int width, height;

	static MeshData meshData;
	static bool loaded;


	Framebuffer(int w, int h, int numColorTexture, bool accessibleDepth, bool hdrEnabled);
    Framebuffer(int w, int h, const std::vector<GLint>& colorFormats, bool accessibleDepth);
	~Framebuffer();

	void deleteTextures();

	void resize(int w, int h);
	void bind(int, GLuint*, bool clear = true);
	void unbind();

	void bindTexture(int slot, int index);
	void bindDepthTexture(int slot);
	
	void blitAll();
	void blitFramebuffer(int index, int x, int y, int dest_width, int dest_height);

	void draw();

private:
    void addColorTexture(int index);
	void addDepthBuffer();
	void addDepthTexture();

	static void load();

};

#endif