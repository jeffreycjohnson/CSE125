#include "NetworkStats.h"
#include "GameObject.h"
#include "Input.h"
#include <iostream>

std::unordered_map<int, int> NetworkStats::clientBytesSent;
std::vector<int> NetworkStats::clients;
bool NetworkStats::debug;

NetworkStats::NetworkStats()
{
}


NetworkStats::~NetworkStats()
{
}

void NetworkStats::registerClients(std::vector<int> clientIDs)
{
	GameObject::AddPreFixedUpdateCallback(NetworkStats::checkDebugButton);
	GameObject::AddPostFixedUpdateCallback(NetworkStats::clearClientBytes);
	for (int client : clientIDs) {
		clientBytesSent[client] = 0;
		clients.push_back(client);
	}
}

void NetworkStats::registerBytesSent(int clientID, int numBytes)
{
	clientBytesSent[clientID] += numBytes;
}

void NetworkStats::clearClientBytes()
{
	for (int client : clients) {
		if (debug)
			std::cout << "sent " << clientBytesSent[client] << " bytes to client " << client << " this tick" << std::endl;
		clientBytesSent[client] = 0;
	}
}

void NetworkStats::checkDebugButton() {
	debug = Input::getKey("n");
}