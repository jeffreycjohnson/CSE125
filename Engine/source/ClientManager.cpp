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
bool objectCreated = false;

const std::vector<int>& ClientManager::initialize(std::string serverIP, std::string port)
{
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
	if (Input::getButtonDown("afterburner")) {
		std::cout << "Client creating object..." << std::endl;
		CreateObjectNetworkData createObj;
		createObj.objectID = 12;
		
		std::vector<char> createBytes;
		createBytes.resize(sizeof(createObj));
		
		memcpy(createBytes.data(), &createObj, sizeof(createObj));

		ClientNetwork::sendBytes(createBytes, CREATE_OBJECT_NETWORK_DATA);
		objectCreated = true;
	}
	else if (Input::getButtonDown("fire")) {
		std::cout << "Destroying object" << std::endl;
	}
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
			GameObject * g = loadScene("assets/ball.dae");
			g->ID = c->objectID;
			std::cout << "Client created object with id " << g->ID << std::endl;
			GameObject::SceneRoot.addChild(g);
		}
		else if (msgType == DESTROY_OBJECT_NETWORK_DATA) {
			DestroyObjectNetworkData * d = (DestroyObjectNetworkData*)received.body.data();
			
		}
	}
}