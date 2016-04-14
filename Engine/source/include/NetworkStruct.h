#ifndef NETWORK_STRUCT_H
#define NETWORK_STRUCT_H

#include <unordered_map>
#include <stdio.h>

#define INPUT_NETWORK_DATA 1
#pragma pack(push, 1)
struct InputNetworkData
{
	unsigned int playerID;

	float yaw, pitch, roll;
	float mouseX, mouseY;
};
#pragma pack(pop)

#define TRANSFORM_NETWORK_DATA 2
#pragma pack(push, 1)
struct TransformNetworkData
{
	unsigned int transformID;

	float px, py, pz;
	float qw, qx, qy, qz;
	float sx, sy, sz;
};
#pragma pack(pop)

class NetworkStruct
{
private:
	static std::unordered_map<int, size_t> structSizes;
public:
	NetworkStruct() {}
	~NetworkStruct() {}

	static size_t sizeOf(int structType);
};

#endif // NETWORK_STRUCT_H