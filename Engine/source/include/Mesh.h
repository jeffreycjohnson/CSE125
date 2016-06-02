#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include "ForwardDecs.h"
#include "Component.h"
#include <unordered_map>
#include <assimp/scene.h>           // Output data structure

struct MeshData {
	GLuint vaoHandle;
    GLsizei indexSize;
    bool wireframe;
	glm::vec3 min, max; // Local min & max
	float radius;
	//BoundingBox boundingBox;
};

struct BoneData {
	std::vector<glm::mat4> boneBindArray;
	std::unordered_map<std::string, int> boneMap;
};

class Mesh : public Component
{
private:
	Mesh();

	void postToNetwork();
	Material* material = nullptr;

public:
	static std::unordered_map<std::string, MeshData> meshMap;
	static std::unordered_map<std::string, BoneData> boneIdMap;

	static void loadMesh(std::string name, const aiMesh* mesh);
	static Mesh* fromCachedMeshData(std::string name);

	static void Dispatch(const std::vector<char> &bytes, int messageType, int messageId);

    std::string name;
	Animation* animationRoot;
	Material* alternateMaterial = nullptr;

    explicit Mesh(std::string);
	~Mesh();

	void setMaterial(Material *mat);
	Material * getMaterial();

	void setGameObject(GameObject* object) override;
	void draw() override;

	std::vector<char> serialize() override;
	void deserializeAndApply(std::vector<char> bytes) override;
	void toggleMaterial();
};

#endif