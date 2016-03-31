#include "ObjectLoader.h"
#include "Mesh.h"
#include "GameObject.h"
#include "Shader.h"
#include "Renderer.h"
#include "Material.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>		// Post processing flags
#include <gtx/quaternion.hpp>

#include "Animation.h"
#include "Light.h"

std::unordered_multimap<std::string, std::function<void(GameObject*)>> componentMap;


int counter = 0;

static GLenum getMapping(aiTextureMapMode mode)
{
    switch (mode)
    {
    case(aiTextureMapMode_Wrap) :
        return GL_REPEAT;
    case(aiTextureMapMode_Clamp) :
        return GL_CLAMP_TO_EDGE;
    case(aiTextureMapMode_Mirror) :
        return GL_MIRRORED_REPEAT;
    case(aiTextureMapMode_Decal) :
        return GL_CLAMP_TO_BORDER;
    default:
        return GL_REPEAT;
    }
}

static std::string getPath(const std::string& name)
{
    auto index = name.find_last_of("\\/");
    if (index == std::string::npos) return name;
    return name.substr(0, index + 1);
}

GameObject* parseNode(const aiScene* scene, aiNode* currentNode, std::string filename, std::unordered_map<std::string, Transform*>& loadingAcceleration, std::map<std::string, Light*>& lights) {
    GameObject* nodeObject = new GameObject();

    //add mesh to this object
    aiVector3D pos;
    aiVector3D scale;
    aiQuaternion rotate;

    currentNode->mTransformation.Decompose(scale, rotate, pos);

    nodeObject->transform.scale(scale.x);
    nodeObject->transform.translate(pos.x, pos.y, pos.z);
    nodeObject->transform.rotate(glm::quat(rotate.w, rotate.x, rotate.y, rotate.z));

    std::string name = currentNode->mName.C_Str();
    if (name == "defaultobject") name = filename + std::to_string(counter);
    nodeObject->setName(name);

    if (lights.count(name))
    {
        nodeObject->addComponent(lights[name]);
    }

    if (currentNode->mNumMeshes > 0) {
        if (!Mesh::meshMap.count(name)) {
            int meshIndex = *currentNode->mMeshes;
            Mesh::loadMesh(name, scene->mMeshes[meshIndex]);
        }

        auto mesh = new Mesh(name);

        auto aMat = scene->mMaterials[scene->mMeshes[*currentNode->mMeshes]->mMaterialIndex];
        auto foundForward = name.find("Forward") != std::string::npos;
        auto foundEmit = name.find("Emit") != std::string::npos;
        Material * mat;
        if (foundForward) {
            mat = new Material(Renderer::getShader(scene->mMeshes[*currentNode->mMeshes]->HasBones() ? FORWARD_PBR_SHADER_ANIM : FORWARD_UNLIT));
            mat->transparent = true;
        }
        else if(foundEmit)
        {
            mat = new Material(Renderer::getShader(FORWARD_EMISSIVE));
            mat->transparent = true;
        }
        else {
            mat = new Material(Renderer::getShader(scene->mMeshes[*currentNode->mMeshes]->HasBones() ? DEFERRED_PBR_SHADER_ANIM : DEFERRED_PBR_SHADER));
            mat->transparent = false;
        }
        if (aMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            aMat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            auto tex = new Texture(getPath(filename) + path.C_Str(), true);
            (*mat)["colorTex"] = tex;
        }
        else
        {
            aiColor3D color(0.f, 0.f, 0.f);
            if (AI_SUCCESS == aMat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
                (*mat)["colorTex"] = new Texture(glm::vec4(color.r, color.g, color.b, 1));
            else
                (*mat)["colorTex"] = new Texture(glm::vec4(1));
        }
        if (aMat->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            aiString path;
            aMat->GetTexture(aiTextureType_NORMALS, 0, &path);
            auto tex = new Texture(getPath(filename) + path.C_Str(), false);
            (*mat)["normalTex"] = tex;
        }
        else
        {
            (*mat)["normalTex"] = new Texture(glm::vec4(0.5, 0.5, 1, 1));
        }
        if (aMat->GetTextureCount(aiTextureType_SPECULAR) > 0)
        {
            aiString path;
            aMat->GetTexture(aiTextureType_SPECULAR, 0, &path);
            auto tex = new Texture(getPath(filename) + path.C_Str(), false);
            (*mat)["matTex"] = tex;
        }
        else
        {
            (*mat)["matTex"] = new Texture(glm::vec4(0, 0.45, 0.7, 1));
        }
        (*mat)["useTextures"] = true;
        mesh->setMaterial(mat);

        nodeObject->addComponent(mesh);
    }

    loadingAcceleration[currentNode->mName.C_Str()] = &nodeObject->transform;

    //load child objects
    for (unsigned int c = 0; c < currentNode->mNumChildren; ++c) {
        nodeObject->addChild(parseNode(scene, currentNode->mChildren[c], filename, loadingAcceleration, lights));
    }

	//To auto load components, use name before period
	//This way we can load e.g. Turret.001 and Turret.002
	// as Turret
	std::string compTypeName;
	std::size_t dividerPos = name.find('.'); //Note: apparently in Collada files '.' becomes '_'
	if (dividerPos != std::string::npos) {
		compTypeName = name.substr(0, dividerPos);
	} else {
		compTypeName = name;
	}

    auto components = componentMap.equal_range(compTypeName);
    if (components.first != componentMap.end())
    {
        while (components.first != components.second) {
            components.first->second(nodeObject);
            ++components.first;
        }
    }
	return nodeObject;
}

void linkRoot(Animation* anim, Transform* currentTransform) {
	if (!currentTransform) return;

	Mesh* currentMesh;
	if ((currentMesh = currentTransform->gameObject->getComponent<Mesh>()) != nullptr) currentMesh->animationRoot = anim;
	for (Transform* child : currentTransform->children) {
		linkRoot(anim, child);
	}
}

GameObject* loadScene(const std::string& filename) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate | aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality |
		aiProcess_FindInvalidData | aiProcess_GenUVCoords | aiProcess_TransformUVCoords |
		aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_SortByPType);

	if (!scene) {
		LOG(importer.GetErrorString());
		throw;
	}

    std::map<std::string, Light*> lights;
    for (unsigned int i = 0; i < scene->mNumLights; i++)
    {
        auto l = scene->mLights[i];

        Light * light;
        if (l->mType == aiLightSource_POINT)
        {
            light = new PointLight();
        }
        else if (l->mType == aiLightSource_DIRECTIONAL)
        {
            light = new DirectionalLight();
        }
        //TODO spotlights
        else
        {
            light = new SpotLight();
        }
        light->color = glm::vec3(l->mColorDiffuse.r, l->mColorDiffuse.g, l->mColorDiffuse.b);
        light->constantFalloff = l->mAttenuationConstant;
        light->linearFalloff = l->mAttenuationLinear;
        light->exponentialFalloff = l->mAttenuationQuadratic;
        lights[l->mName.C_Str()] = light;
    }


	std::unordered_map<std::string, Transform*> loadingAcceleration;

	GameObject* retScene = parseNode(scene, scene->mRootNode, filename, loadingAcceleration, lights);

	if (scene->HasAnimations()) {
		retScene->addComponent(new Animation(scene, loadingAcceleration));
		linkRoot(retScene->getComponent<Animation>(), &retScene->transform);
	}

	return retScene;
}
