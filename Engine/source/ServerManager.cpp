#include "ServerManager.h"

#include <iostream>

#include "ServerInput.h"
#include "ServerNetwork.h"
#include "NetworkStruct.h"

void ServerManager::sendMessages()
{
	ServerNetwork::sendMessage("Hello world!");
}

void ServerManager::receiveMessages()
{
	InputNetworkData* msg;
	int msgType;
	
	char buf[512];
	ServerNetwork::handleClient(buf, &msgType);

	if (msgType == INPUT_NETWORK_DATA)
	{
		msg = (InputNetworkData*)buf;
		ServerInput::deserializeAndApply(*msg);
	}

	// for now, all received messages are client-side input
}