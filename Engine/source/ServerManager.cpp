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

	for (int i = 0; i < responses.size(); i++) {
		if (responses[i].size() < 1) return;

		NetworkResponse final = responses[i].back();
		if (final.messageType == INPUT_NETWORK_DATA)
		{
			msg = (InputNetworkData*)final.body.data();
			std::cout << "playerId: " << msg->playerID << std::endl;
			ServerInput::deserializeAndApply(*msg, msg->playerID);
		}
	}
}