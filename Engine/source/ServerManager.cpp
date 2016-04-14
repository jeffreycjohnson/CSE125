#include "ServerManager.h"

#include <iostream>
#include <vector>

#include "GameObject.h"
#include "ServerInput.h"
#include "ServerNetwork.h"
#include "NetworkStruct.h"

void ServerManager::sendMessages()
{
	TransformNetworkData msg = GameObject::FindByName("player")->transform.serialize();
	ServerNetwork::sendMessage(&msg, TRANSFORM_NETWORK_DATA);
}

void ServerManager::receiveMessages()
{
	InputNetworkData* msg = new InputNetworkData;
	int msgType = -1;

	// for now, all received messages are client-side input
	std::vector<char> buf = ServerNetwork::handleClient(&msgType);
	if (msgType == INPUT_NETWORK_DATA)
	{
		msg = (InputNetworkData*)buf.data();
		ServerInput::deserializeAndApply(*msg);
	}
}