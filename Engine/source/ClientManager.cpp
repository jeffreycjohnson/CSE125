#include "ClientManager.h"

#include <iostream>
#include <string>

#include "ClientNetwork.h"
#include "Input.h"
#include "NetworkStruct.h"
#include "NetworkUtility.h"

void ClientManager::sendMessages()
{
	InputNetworkData inputMessage = Input::serialize();
	char buf[512];
	int contentLength = encodeStruct(&inputMessage, sizeof(InputNetworkData), INPUT_NETWORK_DATA, buf, 512);
	ClientNetwork::sendMessage(buf, contentLength);
}

void ClientManager::receiveMessages()
{
	std::string received = ClientNetwork::receiveMessage();
	std::cout << "recv message " << received << std::endl;
}