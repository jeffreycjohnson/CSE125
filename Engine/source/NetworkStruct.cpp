#include "NetworkStruct.h"

std::unordered_map<int, size_t> NetworkStruct::structSizes = {
	{ CLIENTS_CONN_NETWORK_DATA,   sizeof(ClientsConnNetworkData) },
	{ CREATE_OBJECT_NETWORK_DATA,  sizeof(CreateObjectNetworkData) },
	{ DESTROY_OBJECT_NETWORK_DATA, sizeof(DestroyObjectNetworkData) },
	{ INPUT_NETWORK_DATA,          sizeof(InputNetworkData) },
	{ TRANSFORM_NETWORK_DATA,      sizeof(TransformNetworkData) },
	{ MESH_NETWORK_DATA,           sizeof(MeshNetworkData) },
	{ CAMERA_NETWORK_DATA,         sizeof(CameraNetworkData) },
	{ LIGHT_NETWORK_DATA,          sizeof(LightNetworkData)},
	{ SOUND_NETWORK_DATA,          sizeof(SoundNetworkData) },
	{ CROSSHAIR_NETWORK_DATA,          sizeof(CrosshairNetworkData) },
};

int NetworkStruct::numberOfMessageTypes()
{
	return structSizes.size();
}

size_t NetworkStruct::sizeOf(int structType)
{
	return NetworkStruct::structSizes[structType];
}
