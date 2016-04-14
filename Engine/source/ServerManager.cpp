#include "ServerManager.h"

#include <iostream>

#include "ServerInput.h"
#include "ServerNetwork.h"

void ServerManager::sendMessages()
{
	ServerNetwork::sendMessage("Hello world!");
}

void ServerManager::receiveMessages()
{
	std::string receivedMessage = ServerNetwork::handleClient();
	if (receivedMessage == "") return;

	// for now, all received messages are client-side input
	ServerInput::deserializeStringAndApply(receivedMessage);
}