#ifndef NETWORK_STRUCT_H
#define NETWORK_STRUCT_H

#define INPUT_NETWORK_DATA 1
#pragma pack(push, 1)
struct InputNetworkData
{
	unsigned int playerID;

	float yaw, pitch, roll;
	float mouseX, mouseY;
};
#pragma pack(pop)

#endif // NETWORK_STRUCT_H