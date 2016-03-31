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
	//BoundingBox boundingBox;
};

struct BoneData {
	std::vector<glm::mat4> boneBindArray;
	std::unordered_map<std::string, int> boneMap;
};

class Mesh : public Component
{
	public:
		static std::unordered_map<std::string, MeshData> meshMap;
		static std::unordered_map<std::string, BoneData> boneIdMap;

		static void loadMesh(std::string name, const aiMesh* mesh);

        std::string name;
		Material* material = nullptr;
		Animation* animationRoot;
    
        Mesh(std::string);
		~Mesh();

		void setMaterial(Material *mat);
		void draw() override;
};

#endif