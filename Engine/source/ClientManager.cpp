#include "ClientManager.h"

#include <iostream>
#include <string>

#include "ClientNetwork.h"
#include "GameObject.h"
#include "Input.h"
#include "NetworkStruct.h"
#include "NetworkUtility.h"

bool ClientManager::allClientsConnected;
std::vector<int> ClientManager::clientIDs;
int ClientManager::myClientID;

const std::vector<int>& ClientManager::initialize(std::string serverIP, std::string port)
{
	// setup the connection
	ClientNetwork::SetupTCPConnection(serverIP, port);

	// WAIT FOR THE ALL CLEAR
	while (!ClientManager::allClientsConnected)
	{
		int msgType;
		std::vector<char> received = ClientNetwork::receiveMessage(&msgType);

		if (msgType == CLIENTS_CONN_NETWORK_DATA)
		{
			ClientsConnNetworkData *c = (ClientsConnNetworkData *)received.data();
			ClientManager::allClientsConnected = c->connected;
			if (!ClientManager::allClientsConnected) continue;

			ClientManager::myClientID = c->yourClientID;

			for (int clientID = 0; clientID < c->numClients; clientID++)
			{
				ClientManager::clientIDs.push_back(clientID);
			}

			std::cout << "All clients connected!" << std::endl;
			return ClientManager::clientIDs;
		}
		else if (received.size() != 0)
		{
			std::cerr << "Wrong message received before all clear..." << std::endl;
		}
	}
}

void ClientManager::sendMessages()
{
	if (!ClientManager::allClientsConnected)
	{
		std::cerr << "Tried to send a message before all clients had connected..." << std::endl;
	}

	std::vector<char> bytes = Input::serialize(ClientManager::myClientID);

	ClientNetwork::sendBytes(bytes, INPUT_NETWORK_DATA);
}

void ClientManager::receiveMessages()
{
	TransformNetworkData *tnd;
	int msgType;

	std::vector<char> received = ClientNetwork::receiveMessage(&msgType);
	if (msgType == CLIENTS_CONN_NETWORK_DATA) {
		ClientsConnNetworkData * c = (ClientsConnNetworkData *)received.data();
		if (c->connected)
			ClientManager::allClientsConnected = true;
		std::cout << "All clients connected!" << std::endl;
	}
	else if (msgType == TRANSFORM_NETWORK_DATA)
	{
		tnd = (TransformNetworkData*)received.data();

		std::string playerName = std::string("player_") + std::to_string(tnd->transformID);
		GameObject::FindByName(playerName)->transform.deserializeAndApply(received);
	}
}