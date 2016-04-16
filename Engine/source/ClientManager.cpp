#include "ClientManager.h"

#include <iostream>
#include <string>

#include "ClientNetwork.h"
#include "GameObject.h"
#include "Input.h"
#include "NetworkStruct.h"
#include "NetworkUtility.h"

bool ClientManager::allClientsConnected;

void ClientManager::sendMessages()
{
	if (ClientManager::allClientsConnected) {
		InputNetworkData inputMessage = Input::serialize();
		ClientNetwork::sendMessage(&inputMessage, INPUT_NETWORK_DATA);
	}
	else {
		std::cout << "Waiting for all clients to connect..." << std::endl;
	}
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
		GameObject::FindByName("player")->transform.deserializeAndApply(*tnd);
	}
}