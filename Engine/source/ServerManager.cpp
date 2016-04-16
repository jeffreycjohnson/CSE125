#include "ServerManager.h"

#include <iostream>
#include <vector>

#include "GameObject.h"
#include "ServerInput.h"
#include "ServerNetwork.h"
#include "NetworkStruct.h"

std::vector<int> ServerManager::clientIDs;

void ServerManager::initialize(std::string port, int numberOfClients)
{
	ServerNetwork::setup(port);
	ServerManager::clientIDs = ServerNetwork::startMultiple(numberOfClients);
}

void ServerManager::sendMessages()
{
	TransformNetworkData msg = GameObject::FindByName("player")->transform.serialize();
	ServerNetwork::broadcastMessage(&msg, TRANSFORM_NETWORK_DATA);
}

void ServerManager::receiveMessages()
{
	InputNetworkData* msg = new InputNetworkData;

	// for now, all received messages are client-side input
	std::vector<std::vector<NetworkResponse>> responses = ServerNetwork::selectClients();
	if (responses.size() < 1) return;
	if (responses[0].size() < 1) return;

	NetworkResponse final = responses[0].back();
	if (final.messageType == INPUT_NETWORK_DATA)
	{
		msg = (InputNetworkData*)final.body.data();
		ServerInput::deserializeAndApply(*msg);
	}
}