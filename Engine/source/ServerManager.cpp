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

	// for now, all received messages are client-side input
	std::vector<NetworkResponse> responses = ServerNetwork::handleClient();
	if (responses.size() < 1) return;

	NetworkResponse final = responses.back();
	if (final.messageType == INPUT_NETWORK_DATA)
	{
		msg = (InputNetworkData*)final.body.data();
		ServerInput::deserializeAndApply(*msg);
	}
}