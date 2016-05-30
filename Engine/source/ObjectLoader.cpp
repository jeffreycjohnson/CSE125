#include "ObjectLoader.h"
#include "Mesh.h"
#include "GameObject.h"
#include "Shader.h"
#include "Renderer.h"
#include "Material.h"
#include "Config.h"
#include "Sound.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>		// Post processing flags
#include <gtx/quaternion.hpp>

#include "Animation.h"
#include "Light.h"
#include "GPUEmitter.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

std::unordered_multimap<std::string, std::function<void(GameObject*)>> componentMap;


int counter = 0;

std::string getPath(const std::string& name)
{
    auto index = name.find_last_of("\\/");
    if (index == std::string::npos) return "";
    return name.substr(0, index + 1);
}

static GameObject* parseEmitterNode(const aiScene* scene, aiNode* currentNode) {
	// Extract mesh transform data from Assimp

	GameObject* nodeObject = new GameObject();

	aiVector3D pos;
	aiVector3D scale;
	aiQuaternion rotate;

	currentNode->mTransformation.Decompose(scale, rotate, pos);

	glm::vec3 glmScale(scale.x, scale.y, scale.z);

	nodeObject->transform.setScale(glmScale);
	nodeObject->transform.translate(pos.x, pos.y, pos.z);
	nodeObject->transform.rotate(glm::quat(rotate.w, rotate.x, rotate.y, rotate.z));

	// Particle emitters should be named after a corresponding "particle.ini" file
	// in the hierarchy.   e.g.
	// Emitters
	//   FireEmitter    -->  maps to "assets/particles/Fire.particle.ini"
	std::string name = currentNode->mName.C_Str();
	nodeObject->setName(name);

	auto end = name.find("Emitter");
	if (name != "Emitters" && end != std::string::npos) {
		name.replace(end, name.length() - end, "");

		if (name.length() > 0) {
			std::string path = "assets/particles/" + name + ".particle.ini";

			ConfigFile file(path);
			GPUEmitter* emitter = GPUEmitter::createFromConfigFile(file, nodeObject);
			nodeObject->addComponent(emitter);
			emitter->init();
//			emitter->play();

		}
	}

	// Iterate through the children of the Emitters node & add the emitters
	for (unsigned int c = 0; c < currentNode->mNumChildren; ++c) {
		nodeObject->addChild(parseEmitterNode(scene, currentNode->mChildren[c]));
	}
	
	return nodeObject;

}

static GameObject* parseColliderNode(const aiScene* scene, aiNode* currentNode, bool isStatic = false) {
	GameObject* nodeObject = new GameObject();

	aiVector3D pos;
	aiVector3D scale;
	aiQuaternion rotate;

	currentNode->mTransformation.Decompose(scale, rotate, pos);

	glm::vec3 glmScale(scale.x, scale.y, scale.z);

	nodeObject->transform.setScale(glmScale);
	nodeObject->transform.translate(pos.x, pos.y, pos.z);
	nodeObject->transform.rotate(glm::quat(rotate.w, rotate.x, rotate.y, rotate.z));

	std::string name = currentNode->mName.C_Str();

	if (name.find("BoxCollider") == 0) {
		auto box = new BoxCollider(glm::vec3(0), glm::vec3(2));
		box->setStatic(isStatic);
		box->setAxisAligned(false); // Load OBBs by default
		nodeObject->addComponent(box);
		nodeObject->setName(name);
	}
	else if (name.find("SphereCollider") == 0) {
		auto sphere = new SphereCollider(glm::vec3(0), 1.0f);
		if (glmScale.x != glmScale.y || glmScale.x != glmScale.z || glmScale.y != glmScale.z) {
			LOG("Warning! Loading sphere collider with non-uniform scale!\nTHIS COLLIDER WILL NOT FUNCTION PROPERLY!");
		}
		sphere->setStatic(isStatic);
		nodeObject->addComponent(sphere);
		nodeObject->setName(name);
	}
	else if (name.find("CapsuleCollider") == 0) {
		auto capsule = new CapsuleCollider(glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), 1.f);
		capsule->setStatic(isStatic);
		nodeObject->addComponent(capsule);
		nodeObject->setName(name);
	}
	else {
		nodeObject->setName(name); // Capture name of "Colliders" node
	}

	for (unsigned int c = 0; c < currentNode->mNumChildren; ++c) {
		nodeObject->addChild(parseColliderNode(scene, currentNode->mChildren[c], isStatic));
	}

	return nodeObject;
}

static Mesh* loadMesh(const aiScene* scene, aiNode* currentNode, std::string filename, unsigned int index) {
	int meshIndex = currentNode->mMeshes[index];
	std::string meshName = filename + "/" + scene->mMeshes[meshIndex]->mName.C_Str();
	if (index > 0) meshName = meshName + "/" + std::to_string(index);
	if (!Mesh::meshMap.count(meshName)) {
		Mesh::loadMesh(meshName, scene->mMeshes[meshIndex]);
	}

	auto mesh = new Mesh(meshName);

	auto aMat = scene->mMaterials[scene->mMeshes[meshIndex]->mMaterialIndex];
	aiString matName;
	if (AI_SUCCESS == aMat->Get(AI_MATKEY_NAME, matName))
	{
		Material * mat = new Material(getPath(filename) + matName.C_Str() + ".mat.ini", scene->mMeshes[meshIndex]->HasBones());
		mesh->setMaterial(mat);
	}

	return mesh;
}

static GameObject* parseNode(const aiScene* scene, aiNode* currentNode, std::string filename, std::unordered_map<std::string,
        Transform*>& loadingAcceleration, std::map<std::string, Light*>& lights, bool loadColliders, bool loadEmitters) {
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
		ConfigFile file("config/sounds.ini");
        nodeObject->addComponent(lights[name]);
		Sound::affixSoundToDummy(nodeObject, new Sound("light", true, true, file.getFloat("light","volume"), true));
    }

	for (unsigned int i = 0; i < currentNode->mNumMeshes; i++) {
		auto mesh = loadMesh(scene, currentNode, filename, i);
		if (i == 0) {
			nodeObject->addComponent(mesh);
		}
		else {
			auto child = new GameObject();
			nodeObject->addChild(child);
			child->addComponent(mesh);
		}
	}

    loadingAcceleration[currentNode->mName.C_Str()] = &nodeObject->transform;

    //load child objects
    for (unsigned int c = 0; c < currentNode->mNumChildren; ++c) {
		std::string childName(currentNode->mChildren[c]->mName.C_Str());
		if (childName.find("Colliders") != std::string::npos) {
			// StaticColliders   vs.    Colliders
			bool isStatic = childName.find("Static") == 0;
			if(loadColliders) nodeObject->addChild(parseColliderNode(scene, currentNode->mChildren[c], isStatic));
		}
		else if (childName.find("Emitter") != std::string::npos) {
			if (loadEmitters) nodeObject->addChild(parseEmitterNode(scene, currentNode->mChildren[c])); // TODO: LaserEmitter breaks this :(
		}
		else {
			nodeObject->addChild(parseNode(scene, currentNode->mChildren[c], filename, loadingAcceleration, lights, loadColliders, loadEmitters));
		}
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

GameObject* loadScene(const std::string& filename, bool loadColliders, bool loadEmitters) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_ValidateDataStructure |
		aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FindInstances | 
		aiProcess_FindInvalidData | aiProcess_GenUVCoords | aiProcess_TransformUVCoords |
		aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_SortByPType);

	if (!scene) {
		FATAL(importer.GetErrorString());
	}

    std::map<std::string, Light*> lights;
    for (unsigned int i = 0; i < scene->mNumLights; i++)
    {
        auto l = scene->mLights[i];

        Light * light;
        if (l->mType == aiLightSource_POINT)
        {
            auto pointLight = new PointLight();
            //pointLight->gradient = new Texture("assets/gradient.png");
            light = pointLight;
        }
        else if (l->mType == aiLightSource_DIRECTIONAL)
        {
            light = new DirectionalLight();
        }
        //TODO spotlights
        else
        {
            //light = new SpotLight();
        }
		light->setColor(glm::vec3(l->mColorDiffuse.r, l->mColorDiffuse.g, l->mColorDiffuse.b));
		light->setConstantFalloff(l->mAttenuationConstant);
        light->setLinearFalloff(l->mAttenuationLinear);
		light->setExponentialFalloff(l->mAttenuationQuadratic*15);
        light->setShadowCaster(true);

        lights[l->mName.C_Str()] = light;
    }


	std::unordered_map<std::string, Transform*> loadingAcceleration;

	GameObject* retScene = parseNode(scene, scene->mRootNode, filename, loadingAcceleration, lights, loadColliders, loadEmitters);

	if (scene->HasAnimations()) {
		retScene->addComponent(new Animation(scene, loadingAcceleration));
		linkRoot(retScene->getComponent<Animation>(), &retScene->transform);
	}

	return retScene;
}
