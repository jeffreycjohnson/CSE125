#ifndef NETWORK_STRUCT_H
#define NETWORK_STRUCT_H

#include "ForwardDecs.h"

#include <gtc/quaternion.hpp>

#include <unordered_map>
#include <stdio.h>

class NetworkStruct
{
private:
	static std::unordered_map<int, size_t> structSizes;
public:
	NetworkStruct() {}
	~NetworkStruct() {}

	static size_t sizeOf(int structType);
};

#define CLIENTS_CONN_NETWORK_DATA 0
#pragma pack(push, 1)
struct ClientsConnNetworkData
{
	unsigned int connected;
	unsigned int numClients;
	int yourClientID;
};
#pragma pack(pop)

#define CREATE_OBJECT_NETWORK_DATA 1
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

#define DESTROY_OBJECT_NETWORK_DATA 2
#pragma pack(push, 1)
struct DestroyObjectNetworkData
{
	int objectID; // dummy arg to create object. Server keeps track of ID's.
};
#pragma pack(pop)

#define INPUT_NETWORK_DATA 3
#pragma pack(push, 1)
struct InputNetworkData
{
	int playerID;

	float yaw, pitch, roll;
	float mouseX, mouseY;
};
#pragma pack(pop)

#define TRANSFORM_NETWORK_DATA 4
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

#define MESH_NETWORK_DATA 5
#define MAX_MESH_NAME 64
#pragma pack(push, 1)
struct MeshNetworkData
{
	int objectID;
	char meshName[MAX_MESH_NAME];

	MeshNetworkData(int objectID, std::string meshName)
		: objectID(objectID)
	{
		memset(this->meshName, 0, sizeof(char) * MAX_MESH_NAME);
		strncpy_s(this->meshName, meshName.c_str(), MAX_MESH_NAME - 1);
	}
};
#pragma pack(pop)

#define CAMERA_NETWORK_DATA 6
#pragma pack(push, 1)
struct CameraNetworkData
{
	int objectID;

	CameraNetworkData(int objectID) : objectID(objectID) {}
};
#pragma pack(pop)

#endif // NETWORK_STRUCT_H