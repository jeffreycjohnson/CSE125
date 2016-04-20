#include "ServerManager.h"

#include <iostream>
#include <vector>

#include "GameObject.h"
#include "ServerInput.h"
#include "ServerNetwork.h"
#include "NetworkStruct.h"

std::vector<int> ServerManager::clientIDs;

std::vector<int> ServerManager::initialize(std::string port, int numberOfClients)
{
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

		ServerNetwork::sendMessage(clientID, &cnd, CLIENTS_CONN_NETWORK_DATA);
	}

	return ServerManager::clientIDs;
}

void ServerManager::sendMessages()
{
	// TODO FOUR PLAYERS

	for (auto clientID : ServerManager::clientIDs)
	{
		std::string playerName = std::string("player_") + std::to_string(clientID);
		std::vector<char> bytes = GameObject::FindByName(playerName)->transform.serialize();

		ServerNetwork::broadcastBytes(bytes, TRANSFORM_NETWORK_DATA);
	}

	/*
	std::vector<char> bytes = GameObject::FindByName("player")->transform.serialize();
	ServerNetwork::broadcastMessage(bytes.data(), TRANSFORM_NETWORK_DATA);
	*/
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
		else if (final.messageType == CREATE_OBJECT_NETWORK_DATA) {
			CreateObjectNetworkData * c = (CreateObjectNetworkData*)final.body.data();
			int id = GameObject::createObject();
			c->objectID = id;
			std::cout << "Server created object with ID " << id << std::endl;
			ServerNetwork::broadcastBytes(final.body, CREATE_OBJECT_NETWORK_DATA);
		}
		else if (final.messageType == DESTROY_OBJECT_NETWORK_DATA) {
			DestroyObjectNetworkData * d = (DestroyObjectNetworkData*)final.body.data();
			GameObject::destroyObjectByID(d->objectID);
			ServerNetwork::broadcastBytes(final.body, DESTROY_OBJECT_NETWORK_DATA);
		}
	}
}