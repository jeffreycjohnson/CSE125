#include "ServerManager.h"

#include <iostream>
#include <vector>

#include "GameObject.h"
#include "ServerInput.h"
#include "ServerNetwork.h"
#include "NetworkStruct.h"
#include "NetworkUtility.h"

std::vector<int> ServerManager::clientIDs;
bool ServerManager::isInitialized = false;

std::vector<int> ServerManager::initialize(std::string port, int numberOfClients)
{
	// register hooks in game loop
	GameObject::AddPreFixedUpdateCallback(ServerManager::receiveMessages);
	GameObject::AddPostFixedUpdateCallback(ServerManager::sendMessages);

	// setup the server for listening
	ServerNetwork::setup(port);

	// wait until all clients have registered
	ServerManager::clientIDs = ServerNetwork::startMultiple(numberOfClients);

	// send out all clear
	for (auto clientID : clientIDs)
	{
		ClientsConnNetworkData cnd;
		cnd.connected = 1;
		cnd.numClients = clientIDs.size();
		cnd.yourClientID = clientID;

		std::vector<char> bytes = structToBytes(cnd);

		ServerNetwork::sendBytes(clientID, bytes, CLIENTS_CONN_NETWORK_DATA, clientID);
	}

	ServerManager::isInitialized = true;
	return ServerManager::clientIDs;
}

void ServerManager::sendMessages()
{
	if (!isInitialized) return;

	for (auto clientID : ServerManager::clientIDs)
	{
		std::string playerName = std::string("player_") + std::to_string(clientID);
		GameObject *correctPlayer = GameObject::FindByName(playerName);
		std::vector<char> bytes = correctPlayer->transform.serialize();
		int playerID = correctPlayer->getID();

		ServerNetwork::broadcastBytes(bytes, TRANSFORM_NETWORK_DATA, playerID);
	}
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

			ServerInput::deserializeAndApply(final.body, msg->playerID);
		}
	}
}