#include "Skybox.h"
#include "Renderer.h"
#include <SOIL2.h>
#include "stb_image.h" //note - can get this file from SOIL2 - just put in an include folder in Dependencies for now
#include "Renderer.h"
#include "Camera.h"
#include "Material.h"
#include "RenderPass.h"

#define FLOAT_SIZE 4
#define POSITION_COUNT 3
#define VERTEX_ATTRIB_LOCATION 0
#define VERTEX_COUNT 12
#define INDEX_COUNT 6
#define CUBE_FACES 6
#define SH_COUNT 9
#define sample_count 1

float vertices[VERTEX_COUNT] = { -1, -1, 0,
1, -1, 0,
1,  1, 0,
-1,  1, 0 };
GLuint indices[INDEX_COUNT] = { 0, 1, 2, 0, 2, 3 };

bool Skybox::loaded = false;
MeshData Skybox::meshData;

SkyboxPass::SkyboxPass(Skybox* skybox) : skybox(skybox) {}

void SkyboxPass::render(Camera*)
{
    if (skybox) skybox->draw();
}

void Skybox::load() {
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION);

	GLuint meshBuffer[2];
	glGenBuffers(2, meshBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(float), vertices, GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof(GLuint), indices, GL_STATIC_DRAW);


	int stride = FLOAT_SIZE * (POSITION_COUNT);
	glVertexAttribPointer(VERTEX_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)0);

	meshData.vaoHandle = vao;
	meshData.indexSize = static_cast<GLsizei>(INDEX_COUNT);
    loaded = true;
}

void Skybox::draw() {
	if (!loaded) {
		load();
	}

	material->bind();
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, getTexture());
	Renderer::getShader(SKYBOX_SHADER)["environment"] = 5;


	if (Renderer::gpuData.vaoHandle != meshData.vaoHandle) {
		glBindVertexArray(meshData.vaoHandle);
		Renderer::gpuData.vaoHandle = meshData.vaoHandle;
	}

	glDrawElements(GL_TRIANGLES, meshData.indexSize, GL_UNSIGNED_INT, 0);
}

Skybox::Skybox(std::string imageFiles[6]) {

	ImageData data;
	for (int f = 0; f < CUBE_FACES; ++f) {
		data.imageArray[f] = stbi_loadf(imageFiles[f].c_str(), &data.width[f], &data.height[f], &data.channels[f], 0);
	}

	skyboxTex = loadGLCube(data);
	loadIrradiance(irradianceMatrix, data);

	for (int f = 0; f < CUBE_FACES; ++f) {
		free(data.imageArray[f]);
	}

	material = new Material(&Renderer::getShader(SKYBOX_SHADER));

    applyIrradiance();
    applyTexture(5);
}

void Skybox::applyIrradiance() {
	Renderer::setIrradiance(irradianceMatrix);
}

void Skybox::applyTexture(int slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, getTexture());
	Renderer::setEnvironment(slot, mipmapLevels);
}

GLuint Skybox::getTexture() {
	return skyboxTex;
}


void Skybox::loadIrradiance(glm::mat4(&irradianceMatrix)[3], ImageData& data) {
	//for each cube map
	glm::vec3 irradiance[9];
	const float* currentImage;
	int currentWidth;
	int currentHeight;
	int channels;
	static float shConst[9] = { 0.282095, 0.488603, 0.488603, 0.488603, 1.092548, 1.092548, 1.092548, 0.315392, 0.546274 };

	float xVal, yVal, zVal;

	for (int m = 0; m < CUBE_FACES; ++m) {
		//load image
		//currentImage = SOIL_load_image(imageFiles[m].c_str(), &currentWidth, &currentHeight, &channels, SOIL_LOAD_AUTO);
		currentImage = data.imageArray[m];
		currentWidth = data.width[m];
		currentHeight = data.height[m];
		channels = data.channels[m];

		for (int y = 0; y < currentHeight; ++y) {
			float yPercent = y / (float)currentHeight;

			for (int x = 0; x < currentWidth; ++x) {
				float xPercent = x / (float)currentWidth;

				switch (m) {
				case 0: //rt
					xVal = 1;
					yVal = 2 * yPercent - 1;
					zVal = -(2 * xPercent - 1);
					break;
				case 1: //lf
					xVal = -1;
					yVal = 2 * yPercent - 1;
					zVal = 2 * xPercent - 1;
					break;
				case 2: //up
					xVal = 2 * xPercent - 1;
					yVal = 1;
					zVal = -(2 * yPercent - 1);
					break;
				case 3: //dn
					xVal = 2 * xPercent - 1;
					yVal = -1;
					zVal = 2 * yPercent - 1;
					break;
				case 4: //bk
					xVal = -(2 * xPercent - 1);
					yVal = 2 * yPercent - 1;
					zVal = 1;
					break;
				case 5: //ft
					xVal = (2 * xPercent - 1);
					yVal = 2 * yPercent - 1;
					zVal = -1;
					break;
				}

				float mag = sqrt(xVal*xVal + yVal*yVal + zVal * zVal);
				xVal /= mag;
				yVal /= mag;
				zVal /= mag;

				float theta = acos(zVal / sqrt(xVal*xVal + yVal*yVal + zVal*zVal));

				float currentSH;
				for (int shIndex = 0; shIndex < SH_COUNT; ++shIndex) {
					switch (shIndex) {
					case 0: //0,0
						currentSH = shConst[shIndex];
						break;
					case 1: //1,-1
						currentSH = shConst[shIndex] * yVal;
						break;
					case 2: //1,0
						currentSH = shConst[shIndex] * zVal;
						break;
					case 3: //1,1
						currentSH = shConst[shIndex] * xVal;
						break;
					case 4: //2, -2
						currentSH = shConst[shIndex] * xVal * yVal;
						break;
					case 5: //2, -1
						currentSH = shConst[shIndex] * yVal * zVal;
						break;
					case 6: //2, 0
						currentSH = shConst[shIndex] * (3 * zVal*zVal - 1);
						break;
					case 7: //2, 1
						currentSH = shConst[shIndex] * xVal * zVal;
						break;
					case 8: //2, 2
						currentSH = shConst[shIndex] * (xVal*xVal - yVal*yVal);
						break;
					}
					for (int c = 0; c < 3; ++c) {
						irradiance[shIndex][c] += (currentSH * sin(theta) / (CUBE_FACES*currentWidth*currentHeight)) * (currentImage[(x + y*currentWidth)*channels + c]);
					}
				}
			}
		}
	}

	float c1 = 0.429043;
	float c2 = 0.511664;
	float c3 = 0.743125;
	float c4 = 0.886227;
	float c5 = 0.247708;

	for (int c = 0; c < 3; ++c) {
		irradianceMatrix[c] = glm::mat4(c1*irradiance[8][c], c1*irradiance[4][c], c1*irradiance[7][c], c2*irradiance[3][c],
			c1*irradiance[4][c], -c1*irradiance[8][c], c1*irradiance[5][c], c2*irradiance[1][c],
			c1*irradiance[7][c], c1*irradiance[5][c], c3*irradiance[6][c], c2*irradiance[2][c],
			c2*irradiance[3][c], c2*irradiance[1][c], c2*irradiance[2][c], c4*irradiance[0][c] - c5*irradiance[6][c]);
	}
}







float PI = atanf(1) * 4;

//prev algorithm from http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
//new algorithm from http://cg.informatik.uni-freiburg.de/course_notes/graphics2_04_sampling.pdf slide 22 - old one had incorrect normalization constant
glm::vec2 Hammersley(unsigned int i, unsigned int N) {
	float px = 2;
	int k = i;
	float theta = 0;
	while (k > 0) {
		int a = k % 2;
		theta = theta + (a / px);
		k = int(k / 2);
		px = px * 2;
	}
	return glm::vec2(float(i) / float(N), theta);
}

//sampling angle calculations from http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
//vector calculations from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
glm::vec3 GGX_Sample(glm::vec2 xi, glm::vec3 normal, float a) {
float phi = 2.0 * PI * xi.x;
float cosTheta = sqrt((1.0 - xi.y) / ((a*a - 1.0) * xi.y + 1.0));
float sinTheta = sqrt(1.0 - cosTheta * cosTheta);


glm::vec3 H;
H.x = sinTheta * cos(phi);
H.y = sinTheta * sin(phi);
H.z = cosTheta;


glm::vec3 UpVector = abs(normal.z) < 0.999 ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
glm::vec3 TangentX = normalize(cross(UpVector, normal));
glm::vec3 TangentY = cross(normal, TangentX);

// Tangent to world space
return TangentX * H.x + TangentY * H.y + normal * H.z;
}

float GGX_Visibility(float dotProduct, float k) {
//return 2.0 / (dotProduct + sqrt(k*k + (1 - k*k)*dotProduct*dotProduct)); //More accurate, but slower version

k = k / 2;
return 1.0 / (dotProduct * (1.0 - k) + k);
}


glm::vec3 SpecularBRDF(glm::vec3 lightColor, glm::vec3 normal, glm::vec3 view, glm::vec3 lightDir, float a, float d) {
glm::vec3 halfVec = normalize(view + lightDir);

float dotNL = glm::clamp(glm::dot(normal, lightDir), 0.0f, 1.0f);

float dotNV = glm::clamp(glm::dot(normal, view), 0.0f, 1.0f);
float dotLH = glm::clamp(glm::dot(lightDir, halfVec), 0.0f, 1.0f);

float k = glm::clamp(a + .36f, 0.f, 2.f);
float G = GGX_Visibility(dotNL, k);

return lightColor *(G * G * dotNL);
}

glm::vec3 Skybox::sampleTexture(ImageData& environment, glm::vec3 sampleDirection) {
	int face=0;
	int largestAxis;
	float x = sampleDirection.x;
	float y = sampleDirection.y;
	float z = sampleDirection.z;
	if (abs(x) > abs(y)) {
		if (abs(x) > abs(z)) {
			largestAxis = 0;
		} else {
			largestAxis = 2;
		}
	} else {
		if (abs(y) >= abs(z)) {
			largestAxis = 1;
		}
		else {
			largestAxis = 2;
		}
	}

	int s, t;
	glm::vec3 retVal;
	
	switch (largestAxis) {
		case 0:
			y /= abs(x);
			z /= abs(x);
			y = y*0.5 + 0.5;
			z = z*0.5 + 0.5;
			face = (x > 0) ? 0 : 1;
			s = (environment.width[face]-1) * ((x>0) ? 1-z : z);
			t = (environment.height[face]-1) * y;
			for (int c = 0; c < 3; ++c) {
				retVal[c] = environment.imageArray[face][(s + t*environment.width[face])*environment.channels[face] + c];
			}
			break;
		case 1:
			x /= abs(y);
			z /= abs(y);
			x = x*0.5 + 0.5;
			z = z*0.5 + 0.5;
			face = (y < 0) ? 2 : 3;
			s = (environment.width[face] - 1) * x;
			t = (environment.height[face] - 1) * ((y>0) ? 1-z : z);
			for (int c = 0; c < 3; ++c) {
				retVal[c] = environment.imageArray[face][(s + t*environment.width[face])*environment.channels[face] + c];
			}
			break;
		case 2:
			x /= abs(z);
			y /= abs(z);
			x = x*0.5 + 0.5;
			y = y*0.5 + 0.5;
			face = (z > 0) ? 4 : 5;
			s = (environment.width[face]-1) * ((z<0) ? 1-x : x);
			t = (environment.height[face]-1) * y;
			for (int c = 0; c < 3; ++c) {
				retVal[c] = environment.imageArray[face][(s + t*environment.width[face])*environment.channels[face] + c];
			}
			break;
	}
	return retVal;
}

//generates sample directions, sets up the values, calls the BRDF, then accumulates resulting colors
glm::vec3 Skybox::SpecularEnvMap(glm::vec3 normal, float a, ImageData& environment) {
	glm::vec3 color = glm::vec3(0, 0, 0);
	glm::vec3 & view = normal;
	glm::vec3 lightDir_Main = normal;//reflect(-view, normal);
	float weight=0;
	for (int s = 0; s<sample_count; ++s) {
		glm::vec2 xi = Hammersley(s, sample_count);
		glm::vec3 lightDir = GGX_Sample(xi, lightDir_Main, a);
		glm::vec3 lightColor = sampleTexture(environment, lightDir);
		float val = 1;//glm::clamp(glm::dot(normal, lightDir), 0.0f, 1.0f);
		color += val * SpecularBRDF(lightColor, normal, view, lightDir, a, 0);
		weight += val;
	}
	color /= weight;
	return color;
}

GLuint Skybox::loadGLCube(ImageData& data) {
	GLuint cubeTextureHandle;
	glGenTextures(1, &cubeTextureHandle);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureHandle);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	int mipmapLevel = (int)(log(data.width[0]) / log(2)) + 1;

	this->mipmapLevels = mipmapLevel-1;

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipmapLevel-1);

	for (int m = 0; m < CUBE_FACES; ++m) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + m, 0, GL_RGB16F, data.width[m], data.height[m], 0, GL_RGB, GL_FLOAT, data.imageArray[m]);
		for (int mip = 1; mip < mipmapLevel; ++mip) {
			int width = data.width[m] / pow(2, mip);
			int height = data.height[m] / pow(2, mip);
			float* tmpImage = new float[3 * width * height];

			for (int t = 0; t < height; ++t) {
				for (int s = 0; s < width; ++s) {

					float xVal, yVal, zVal;
					float yPercent = (t+0.5f) / (float)height;
					float xPercent = (s+0.5f) / (float)width;

					switch (m) {
					case 0: //rt
						xVal = 1;
						yVal = 2 * yPercent - 1;
						zVal = -(2 * xPercent - 1);
						break;
					case 1: //lf
						xVal = -1;
						yVal = 2 * yPercent - 1;
						zVal = (2 * xPercent - 1);
						break;
					case 2: //up
						xVal = (2 * xPercent - 1);
						yVal = -1;
						zVal = (2 * yPercent - 1);
						break;
					case 3: //dn
						xVal = (2 * xPercent - 1);
						yVal = 1;
						zVal = -(2 * yPercent - 1);
						break;
					case 4: //bk
						xVal = (2 * xPercent - 1);
						yVal = 2 * yPercent - 1;
						zVal = 1;
						break;
					case 5: //ft
						xVal = -(2 * xPercent - 1);
						yVal = 2 * yPercent - 1;
						zVal = -1;
						break;
					}

					float a = mip / (float)(mipmapLevel - 1);
					a = a*a;//*a*a;
					glm::vec3 color = SpecularEnvMap(glm::normalize(glm::vec3(xVal, yVal, zVal)), a, data);
					for (int c = 0; c < 3; ++c) {
						tmpImage[3 * (s + t*width) + c] = color[c];
					}
				}
			}
			
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + m, mip, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, tmpImage);

			delete[] tmpImage;
		}
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	return cubeTextureHandle;
}