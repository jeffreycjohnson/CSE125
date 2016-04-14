#include "ClientManager.h"

#include <iostream>
#include <string>

#include "ClientNetwork.h"
#include "GameObject.h"
#include "Input.h"
#include "NetworkStruct.h"
#include "NetworkUtility.h"

void ClientManager::sendMessages()
{
	InputNetworkData inputMessage = Input::serialize();
	ClientNetwork::sendMessage(&inputMessage, INPUT_NETWORK_DATA);
}

void ClientManager::receiveMessages()
{
	TransformNetworkData *tnd;
	int msgType;

	std::vector<char> received = ClientNetwork::receiveMessage(&msgType);
	if (msgType == TRANSFORM_NETWORK_DATA)
	{
		tnd = (TransformNetworkData*)received.data();
		GameObject::FindByName("player")->transform.deserializeAndApply(*tnd);
	}
}