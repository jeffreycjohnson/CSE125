#ifndef NETWORK_STRUCT_H
#define NETWORK_STRUCT_H

#include "ForwardDecs.h"

#include <gtc/quaternion.hpp>

#include <unordered_map>
#include <stdio.h>

enum {
	CLIENTS_CONN_NETWORK_DATA,
	CREATE_OBJECT_NETWORK_DATA,
	DESTROY_OBJECT_NETWORK_DATA,
	INPUT_NETWORK_DATA,
	TRANSFORM_NETWORK_DATA,
	MESH_NETWORK_DATA,
	CAMERA_NETWORK_DATA,
	LIGHT_NETWORK_DATA
};

class NetworkStruct
{
private:
	static std::unordered_map<int, size_t> structSizes;
public:
	NetworkStruct() {}
	~NetworkStruct() {}

	static size_t sizeOf(int structType);
};

#pragma pack(push, 1)
struct ClientsConnNetworkData
{
	unsigned int connected;
	unsigned int numClients;
	int yourClientID;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CreateObjectNetworkData
{
	int objectID;

	CreateObjectNetworkData(int objectID)
		: objectID(objectID)
	{
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DestroyObjectNetworkData
{
	int objectID; // dummy arg to create object. Server keeps track of ID's.
};
#pragma pack(pop)

#pragma pack(push, 1)
struct InputNetworkData
{
	int playerID;

	float yaw, pitch, roll;
	float mouseX, mouseY;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TransformNetworkData
{
	int objectID;
	int parentID;

	float px, py, pz;
	float qw, qx, qy, qz;
	float sx, sy, sz;

	TransformNetworkData() {}
	TransformNetworkData(int objectID, int parentID,
		glm::vec3 position, glm::quat rotation, glm::vec3 scaleFactor)
	{
		this->objectID = objectID;
		this->parentID = parentID;

		this->px = position.x;
		this->py = position.y;
		this->pz = position.z;

		this->qw = rotation.w;
		this->qx = rotation.x;
		this->qy = rotation.y;
		this->qz = rotation.z;

		this->sx = scaleFactor.x;
		this->sy = scaleFactor.y;
		this->sz = scaleFactor.z;
	}
};
#pragma pack(pop)

#define MAX_MESH_NAME 64
#define MAX_MATERIAL_NAME 64
#pragma pack(push, 1)
struct MeshNetworkData
{
	int objectID;
	char meshName[MAX_MESH_NAME];
	char materialName[MAX_MATERIAL_NAME];
	bool hasAnimations;

	MeshNetworkData(int objectID, std::string meshName, std::string materialName, bool hasAnimations)
		: objectID(objectID), hasAnimations(hasAnimations)
	{
		memset(this->meshName, 0, sizeof(char) * MAX_MESH_NAME);
		strncpy_s(this->meshName, meshName.c_str(), MAX_MESH_NAME - 1);
		memset(this->materialName, 0, sizeof(char) * MAX_MATERIAL_NAME);
		strncpy_s(this->materialName, materialName.c_str(), MAX_MATERIAL_NAME - 1);
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CameraNetworkData
{
	int objectID;
	int clientID;
	CameraNetworkData(int objectID) : objectID(objectID) {}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct LightNetworkData
{
	int objectID;

	int lightType;
	float colorr, colorg, colorb;
	bool shadowCaster;
	float radius;
	float constantFalloff, linearFalloff, exponentialFalloff;

	LightNetworkData(
		int objectID, int lightType, glm::vec3 color, bool shadowCaster, float radius,
		float constantFalloff, float linearFalloff, float exponentialFalloff) : 
		objectID(objectID), lightType(lightType), shadowCaster(shadowCaster), radius(radius),
		constantFalloff(constantFalloff), linearFalloff(linearFalloff), exponentialFalloff(exponentialFalloff)
		{
			this->colorr = color.r;
			this->colorg = color.g;
			this->colorb = color.b;
	}
};
#pragma pack(pop)

#endif // NETWORK_STRUCT_H