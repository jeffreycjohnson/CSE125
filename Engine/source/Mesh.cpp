#include "Mesh.h"
#include "GameObject.h"
#include "Shader.h"
#include "Renderer.h"
#include "NetworkManager.h"
#include "NetworkUtility.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>		// Post processing flags

#include <iostream>	

#include "Animation.h"
#include "Material.h"

#define POSITION_COUNT 3
#define NORMAL_COUNT 3
#define TANGENT_COUNT 3
#define TEX_COORD_COUNT 2
#define BONE_ID_COUNT 4
#define BONE_WEIGHT_COUNT 4

#define VERTEX_ATTRIB_LOCATION 0
#define NORMAL_ATTRIB_LOCATION 1
#define TEX_COORD_0_ATTRIB_LOCATION 2
#define TANGENT_ATTRIB_LOCATION 3
#define BITANGENT_ATTRIB_LOCATION 4
#define BONE_WEIGHT_ATTRIB_LOCATION 5
#define BONE_ID_ATTRIB_LOCATION 6

std::unordered_map<std::string, MeshData> Mesh::meshMap;
std::unordered_map<std::string, BoneData>  Mesh::boneIdMap;

Mesh* Mesh::fromCachedMeshData(std::string name)
{
	auto iter = Mesh::meshMap.find(name);
	if (iter == Mesh::meshMap.end())
	{
		FATAL("Can only create mesh from cached data if data in cache");
	}

	Mesh *created = new Mesh;
	created->name = name;

	return created;
}

void Mesh::Dispatch(const std::vector<char> &bytes, int messageType, int messageId)
{
	MeshNetworkData mnd = structFromBytes<MeshNetworkData>(bytes);

	GameObject *go = GameObject::FindByID(messageId);
	if (go == nullptr)
	{
		FATAL("Cannot set mesh of nonexistant gameobject");
	}

	Mesh * cachedMesh = Mesh::fromCachedMeshData(std::string(mnd.meshName));
	cachedMesh->setMaterial(new Material(mnd.materialName, mnd.hasAnimations));
	std::cout << mnd.materialName << std::endl;
	go->addComponent(cachedMesh);
}

Mesh::Mesh() {}

Mesh::Mesh(std::string name) : name(name) {
	if (Mesh::meshMap.find(name) == Mesh::meshMap.end()) {
		FATAL(name.c_str());
	}
}

Mesh::~Mesh() {

}

void Mesh::draw() {
	MeshData& currentEntry = meshMap.at(name);
	if (Renderer::gpuData.vaoHandle != currentEntry.vaoHandle) {
		glBindVertexArray(currentEntry.vaoHandle);
		Renderer::gpuData.vaoHandle = currentEntry.vaoHandle;
	}

	//TODO move
	if(gameObject) Renderer::setModelMatrix(gameObject->transform.getTransformMatrix());
    else Renderer::setModelMatrix(glm::mat4());

	if ((Renderer::currentShader == &Renderer::getShader(FORWARD_PBR_SHADER_ANIM) ||
        Renderer::currentShader == &Renderer::getShader(DEFERRED_PBR_SHADER_ANIM) ||
        Renderer::currentShader == &Renderer::getShader(SHADOW_SHADER_ANIM))&& animationRoot) {
		BoneData & meshBoneData = boneIdMap.at(name);

		for (AnimNodeData node : animationRoot->getAnimationData()) {
			auto bone = meshBoneData.boneMap.find(node.name);
			if (bone == meshBoneData.boneMap.end()) continue;
			int id = bone->second;


			glm::mat4 transformMatrix = node.object->getTransformMatrix() * meshBoneData.boneBindArray[id];


			(*Renderer::currentShader)[std::string("bone_Matrix[") + std::to_string(id) + "]"] = transformMatrix;
		}
	}

    if(currentEntry.wireframe)
    {
        glDrawElements(GL_LINES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawElements(GL_TRIANGLES, currentEntry.indexSize, GL_UNSIGNED_INT, 0);
    }
}

void Mesh::setGameObject(GameObject* object)
{
	Component::setGameObject(object);
	postToNetwork();
}

std::vector<char> Mesh::serialize()
{
	std::string materialName = "assets/DefaultMaterial.mat.ini";
	if (material != nullptr)
	{
		materialName = material->getWatcherFileName();
	}

	MeshNetworkData mnd = MeshNetworkData(gameObject->getID(), name, materialName, false);
	return structToBytes(mnd);
}

void Mesh::deserializeAndApply(std::vector<char> bytes)
{
	MeshNetworkData mnd = structFromBytes<MeshNetworkData>(bytes);
	auto iter = Mesh::meshMap.find(mnd.meshName);
	if (iter == Mesh::meshMap.end())
	{
		FATAL("Can only create mesh from cached data if data in cache");
	}

	this->name = std::string(mnd.meshName);
}

void Mesh::toggleMaterial()
{
	Material * tmpMat = this->getMaterial();
	this->setMaterial(alternateMaterial);
	this->alternateMaterial = tmpMat;
}

void Mesh::postToNetwork()
{
	if (NetworkManager::getState() != SERVER_MODE) return;

	GameObject *my = gameObject;
	if (my == nullptr)
	{
		//std::cerr << "Mesh with no attached game object modified??" << std::endl;
		return;
	}

	std::cout << "sent " << this->name << std::endl;
	NetworkManager::PostMessage(serialize(), MESH_NETWORK_DATA, my->getID());
}

void Mesh::setMaterial(Material *mat) 
{
	material = mat;
	postToNetwork();
}

Material * Mesh::getMaterial()
{
	return material;
}

bool boneWeightSort(std::pair<int, float> bone1, std::pair<int, float> bone2) {
	return bone1.second > bone2.second;
}

void Mesh::loadMesh(std::string name, const aiMesh* mesh) {

	std::vector<float> megaArray;
	std::vector<int> idArray;
	std::vector<int> indexArray;

	// Calculate min/max points (these get sent to the Octree when a scene is loaded)
	float xmin, ymin, zmin;
	float xmax, ymax, zmax;
	xmax = ymax = zmax = -FLT_MAX;
	xmin = ymin = zmin = FLT_MAX;

	bool enabledTexCoord[8];
	for (int t = 0; t < 8; ++t) {
		enabledTexCoord[t] = mesh->HasTextureCoords(t);
	}

    bool hasTangents = mesh->HasTangentsAndBitangents();
    bool hasNormals = mesh->HasNormals();

	std::pair<glm::ivec4, glm::vec4> *boneResults;

	if (mesh->HasBones()) {


		Mesh::boneIdMap[name] = BoneData();
		Mesh::boneIdMap[name].boneBindArray = std::vector<glm::mat4>(mesh->mNumBones);


		boneResults = new std::pair<glm::ivec4, glm::vec4>[mesh->mNumVertices];
		std::vector<std::pair<int, float>> *boneData = new std::vector<std::pair<int, float>>[mesh->mNumVertices];

		for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
			Mesh::boneIdMap[name].boneMap[mesh->mBones[b]->mName.C_Str()] = b;
			for (int matIndex = 0; matIndex < 16; ++matIndex) {
				//Assimp matrices are row major, glm & opengl are column major, so we need to convert here
				Mesh::boneIdMap[name].boneBindArray[b][matIndex % 4][matIndex / 4] = mesh->mBones[b]->mOffsetMatrix[matIndex / 4][matIndex % 4];
			}

			for (unsigned int w = 0; w < mesh->mBones[b]->mNumWeights; ++w) {
				boneData[mesh->mBones[b]->mWeights[w].mVertexId].push_back(std::make_pair(b, mesh->mBones[b]->mWeights[w].mWeight));
			}
		}

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			std::sort(boneData[i].begin(), boneData[i].end(), boneWeightSort);
			glm::ivec4 bones(0, 0, 0, 0);
			glm::vec4 weights(0, 0, 0, 0);
			for (int d = 0; d < 4 && d < boneData[i].size(); ++d) {
				bones[d] = boneData[i][d].first;
				weights[d] = boneData[i][d].second;
			}
			weights = weights / (weights[0] + weights[1] + weights[2] + weights[3]);
			boneResults[i] = std::make_pair(bones, weights);
		}

		delete[] boneData;
	}


	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		for (int p = 0; p < POSITION_COUNT; ++p) {
			auto vertex = mesh->mVertices[i];
			xmin = std::fminf(vertex.x, xmin);
			ymin = std::fminf(vertex.y, ymin);
			zmin = std::fminf(vertex.z, zmin);
			xmax = std::fmaxf(vertex.x, xmax);
			ymax = std::fmaxf(vertex.y, ymax);
			zmax = std::fmaxf(vertex.z, zmax);
			megaArray.push_back(vertex[p]);
		}
        if (hasNormals) {
            for (int p = 0; p < NORMAL_COUNT; ++p) {
                megaArray.push_back(mesh->mNormals[i][p]);
            }
        }
		if (enabledTexCoord[0]) {
			for (int p = 0; p < TEX_COORD_COUNT; ++p) {
				megaArray.push_back(mesh->mTextureCoords[0][i][p]);
			}
		}
		if (hasTangents) {
			for (int p = 0; p < TANGENT_COUNT; ++p) {
				megaArray.push_back(mesh->mTangents[i][p]);
			}
			for (int p = 0; p < TANGENT_COUNT; ++p) {
				megaArray.push_back(mesh->mBitangents[i][p]);
			}
		}
		if (mesh->HasBones()) {
			for (int p = 0; p < BONE_WEIGHT_COUNT; ++p) {
				megaArray.push_back(boneResults[i].second[p]);
			}
			for (int p = 0; p < BONE_ID_COUNT; ++p) {
				idArray.push_back(boneResults[i].first[p]);
			}
		}
	}

	if (mesh->HasBones()) delete[] boneResults;


	for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        unsigned indexCount = mesh->mPrimitiveTypes == aiPrimitiveType_LINE ? 2 : 3;
		for (unsigned int p = 0; p < indexCount; ++p) {
			indexArray.push_back(mesh->mFaces[f].mIndices[p]);
		}
	}


	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint meshBuffer[3];
	glGenBuffers(3, meshBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, megaArray.size() * sizeof(float), &(megaArray[0]), GL_STATIC_DRAW);



	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexArray.size() * sizeof(int), &(indexArray[0]), GL_STATIC_DRAW);


	int stride = sizeof(float) * POSITION_COUNT;
    if (hasNormals) stride += sizeof(float) * NORMAL_COUNT;
	if (hasTangents) stride += sizeof(float) * (2 * TANGENT_COUNT);
	if (enabledTexCoord[0]) stride += sizeof(float) * TEX_COORD_COUNT;
	if (mesh->HasBones()) stride += sizeof(float) * BONE_WEIGHT_COUNT;

	uintptr_t currentOffset = 0;

    glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION);
	glVertexAttribPointer(VERTEX_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (sizeof(float) * 3);
    if (hasNormals) {
        glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
        glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (sizeof(float) * 3);
    }
	if (enabledTexCoord[0]) {
        glEnableVertexAttribArray(TEX_COORD_0_ATTRIB_LOCATION);
		glVertexAttribPointer(TEX_COORD_0_ATTRIB_LOCATION, 2, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (sizeof(float) * 2);
	}
	if (hasTangents) {
        glEnableVertexAttribArray(TANGENT_ATTRIB_LOCATION);
        glEnableVertexAttribArray(BITANGENT_ATTRIB_LOCATION);
		glVertexAttribPointer(TANGENT_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (sizeof(float) * 3);
		glVertexAttribPointer(BITANGENT_ATTRIB_LOCATION, 3, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (sizeof(float) * 3);
	}
	if (mesh->HasBones()) {
        glEnableVertexAttribArray(BONE_ID_ATTRIB_LOCATION);
        glEnableVertexAttribArray(BONE_WEIGHT_ATTRIB_LOCATION);
		glVertexAttribPointer(BONE_WEIGHT_ATTRIB_LOCATION, 4, GL_FLOAT, false, stride, (GLvoid*)currentOffset); currentOffset += (sizeof(float) * 4);

		glBindBuffer(GL_ARRAY_BUFFER, meshBuffer[2]);
		glBufferData(GL_ARRAY_BUFFER, idArray.size() * sizeof(int), &(idArray[0]), GL_STATIC_DRAW);
		glVertexAttribIPointer(BONE_ID_ATTRIB_LOCATION, 4, GL_INT, sizeof(int) * 4, (GLvoid*)0);
	}

	MeshData meshData;
	meshData.vaoHandle = vao;
	meshData.indexSize = static_cast<GLsizei>(indexArray.size());
    meshData.wireframe = mesh->mPrimitiveTypes == aiPrimitiveType_LINE;
	meshData.min = glm::vec3(xmin, ymin, zmin);
	meshData.max = glm::vec3(xmax, ymax, zmax);
	meshData.radius = glm::length(glm::vec3(fmaxf(fabsf(xmin), fabsf(xmax)), fmaxf(fabsf(ymin), fabsf(ymax)), fmaxf(fabsf(zmin), fabsf(zmax))));

	Mesh::meshMap[name] = meshData;

}
