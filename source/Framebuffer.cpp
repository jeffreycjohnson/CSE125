#include "Framebuffer.h"
#include "Renderer.h"
#include "Camera.h"
#include <gtc/matrix_transform.hpp>


Framebuffer::Framebuffer(int w, int h, int numColorTextures, bool accessibleDepth, bool hdrEnabled)
        : accessibleDepth(accessibleDepth), numColorTex(numColorTextures), hdrEnabled(hdrEnabled), width(w), height(h) {
    colorFormats.resize(numColorTextures);
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);

	colorTex = new GLuint[numColorTex];

	for (int x = 0; x < numColorTex; ++x) {
        colorFormats[x] = (hdrEnabled) ? GL_RGBA16F : GL_RGBA;
		addColorTexture(x);
	}

	if (accessibleDepth) {
		addDepthTexture();
	} else {
		addDepthBuffer();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::Framebuffer(int w, int h, const std::vector<GLint>& colorFormats, bool accessibleDepth)
        : accessibleDepth(accessibleDepth), numColorTex(colorFormats.size()), colorFormats(colorFormats), width(w), height(h) {
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    colorTex = new GLuint[numColorTex];

    for (int x = 0; x < numColorTex; ++x) {
        addColorTexture(x);
    }

    if (accessibleDepth) {
        addDepthTexture();
    }
    else {
        addDepthBuffer();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::addColorTexture(int index) {
	glGenTextures(1, &(colorTex[index]) );
	glBindTexture(GL_TEXTURE_2D, colorTex[index]);

	glTexImage2D(GL_TEXTURE_2D, 0, colorFormats[index], width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, colorTex[index], 0);
}

void Framebuffer::addDepthTexture() {
	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// For Hardware PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL); 

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
}

void Framebuffer::addDepthBuffer() {
	glGenRenderbuffers(1, &depthTex);
	glBindRenderbuffer(GL_RENDERBUFFER, depthTex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthTex);
}

Framebuffer::~Framebuffer() {
	deleteTextures();
	glDeleteFramebuffers(1, &id);
	delete[] colorTex;
}

void Framebuffer::deleteTextures()
{
	glDeleteTextures(numColorTex, colorTex);
	if (accessibleDepth) {
		glDeleteTextures(1, &depthTex);
	} else {
		glDeleteRenderbuffers(1, &depthTex);
	}
}

void Framebuffer::bind(int bufferCount, GLuint *buffersToDraw, bool clear) {

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glDrawBuffers(bufferCount, buffersToDraw);
	if(clear) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//TODO use Renderer::resize()?
	glViewport(0, 0, width, height);
	glm::mat4 perspective = glm::perspective(Renderer::currentCamera->getFOV(), width / (float)height, NEAR_DEPTH, FAR_DEPTH);
	Renderer::updatePerspective(perspective);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, Renderer::getWindowWidth(), Renderer::getWindowHeight());
	glm::mat4 perspective = glm::perspective(Renderer::currentCamera->getFOV(), Renderer::getWindowWidth() / (float)Renderer::getWindowHeight(), NEAR_DEPTH, FAR_DEPTH);
	Renderer::updatePerspective(perspective);
}

void Framebuffer::bindTexture(int slot, int colorIndex) {
	glActiveTexture(GL_TEXTURE0 + slot);
	GLuint tex = colorTex[colorIndex];
	glBindTexture(GL_TEXTURE_2D, tex);
}

void Framebuffer::bindDepthTexture(int slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	GLuint tex = depthTex;
	glBindTexture(GL_TEXTURE_2D, tex);
}

void Framebuffer::blitFramebuffer(int index, int x, int y, int destW, int destH) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
	glBlitFramebuffer(0, 0, width, height, x, y, x+destW, y+destH, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void Framebuffer::blitAll() {
	for (int index = 0; index < this->numColorTex; ++index) {
		blitFramebuffer(index, index * 300, 0, 300, 300);
	}
}

void Framebuffer::resize(int w, int h) {
	width = w;
	height = h;

	deleteTextures();

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	for (int x = 0; x < numColorTex; ++x) {
		addColorTexture(x);
	}

	if (accessibleDepth) {
		addDepthTexture();
	} else {
		addDepthBuffer();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


#define VERTEX_COUNT ((3+2) * 4)
float fbo_vertices[VERTEX_COUNT] = { -1, -1, 0, 0, 0,
1, -1, 0, 1, 0,
1,  1, 0, 1, 1,
-1,  1, 0, 0, 1 };

#define INDEX_COUNT 6
GLuint fbo_indices[INDEX_COUNT] = { 0, 1, 2, 0, 2, 3 };

#define FLOAT_SIZE 4

#define POSITION_COUNT 3
#define TEX_COORD_COUNT 2

#define VERTEX_ATTRIB_LOCATION 0
#define TEX_COORD_0_ATTRIB_LOCATION 2

MeshData Framebuffer::meshData;
bool Framebuffer::loaded = false;

void Framebuffer::load() {

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION);
	glEnableVertexAttribArray(TEX_COORD_0_ATTRIB_LOCATION);

	GLuint meshBuffer[2];
	glGenBuffers(2, meshBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(float), fbo_vertices, GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof(GLuint), fbo_indices, GL_STATIC_DRAW);


	int stride = FLOAT_SIZE * (POSITION_COUNT + TEX_COORD_COUNT);
	glVertexAttribPointer(VERTEX_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)0);
	glVertexAttribPointer(TEX_COORD_0_ATTRIB_LOCATION, 2, GL_FLOAT, false, stride, (GLvoid*)(FLOAT_SIZE * 3));

	meshData.vaoHandle = vao;
	meshData.indexSize = static_cast<GLsizei>(INDEX_COUNT);
	
	loaded = true;
}

void Framebuffer::draw() {
	if (!loaded) {
		load();
	}

	if (Renderer::gpuData.vaoHandle != meshData.vaoHandle) {
		glBindVertexArray(meshData.vaoHandle);
		Renderer::gpuData.vaoHandle = meshData.vaoHandle;
	}

	glDrawElements(GL_TRIANGLES, meshData.indexSize, GL_UNSIGNED_INT, 0);
}