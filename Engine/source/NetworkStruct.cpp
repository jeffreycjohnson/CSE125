#include "NetworkStruct.h"

std::unordered_map<int, size_t> NetworkStruct::structSizes = {
	{ INPUT_NETWORK_DATA, sizeof(InputNetworkData) },
	{ TRANSFORM_NETWORK_DATA, sizeof(TransformNetworkData) },
};

size_t NetworkStruct::sizeOf(int structType)
{
	return NetworkStruct::structSizes[structType];
}