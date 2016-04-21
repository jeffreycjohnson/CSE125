#include "ClientManager.h"

#include <iostream>
#include <string>

#include "ClientNetwork.h"
#include "GameObject.h"
#include "Input.h"
#include "NetworkStruct.h"
#include "NetworkUtility.h"
#include "ObjectLoader.h"

bool ClientManager::allClientsConnected;
std::vector<int> ClientManager::clientIDs;
int ClientManager::myClientID;
int ClientManager::lastObjectCreated; // This is just for now to show delete on the last obj that was created.

const std::vector<int>& ClientManager::initialize(std::string serverIP, std::string port)
{
	// register hooks in the game loop
	GameObject::AddPreFixedUpdateCallback(ClientManager::receiveMessages);
	GameObject::AddPostFixedUpdateCallback(ClientManager::sendMessages);

	// setup the connection
	ClientNetwork::setup(serverIP, port);

	// WAIT FOR THE ALL CLEAR
	while (!ClientManager::allClientsConnected)
	{
		std::vector<NetworkResponse> messages = ClientNetwork::receiveMessages();
		
		for (auto& response : messages)
		{
			if (response.messageType == CLIENTS_CONN_NETWORK_DATA)
			{
				ClientsConnNetworkData *c = (ClientsConnNetworkData *)response.body.data();
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
			else if (response.body.size() != 0)
			{
				std::cerr << "Wrong message received before all clear..." << std::endl;
			}
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

	std::vector<NetworkResponse> receivedMessages = ClientNetwork::receiveMessages();

	for (auto& received : receivedMessages)
	{
		int& msgType = received.messageType;
		if (msgType == CLIENTS_CONN_NETWORK_DATA) {
			std::cerr << "RECEIVED ALL CLEAR MESSAGE AFTER SERVER INITIALIZATION" << std::endl;
		}
		else if (msgType == TRANSFORM_NETWORK_DATA)
		{
			tnd = (TransformNetworkData*)received.body.data();

			std::string playerName = std::string("player_") + std::to_string(tnd->transformID);
			GameObject::FindByName(playerName)->transform.deserializeAndApply(received.body);
		}
		else if (msgType == CREATE_OBJECT_NETWORK_DATA) {
			CreateObjectNetworkData * c = (CreateObjectNetworkData*)received.body.data();
			GameObject::createObject(c->objectID);

			GameObject * g = GameObject::SceneRoot.FindByID(c->objectID);
			
			std::cout << "Client created object with id " << g->getID() << std::endl;
			g->transform.setPosition(g->getID(), -1, 0);
			ClientManager::lastObjectCreated = g->getID();
		}
		else if (msgType == DESTROY_OBJECT_NETWORK_DATA) {
			DestroyObjectNetworkData * d = (DestroyObjectNetworkData*)received.body.data();
			GameObject * g = GameObject::SceneRoot.FindByID(d->objectID);
			GameObject::destroyObjectByID(d->objectID);
		}
	}
}