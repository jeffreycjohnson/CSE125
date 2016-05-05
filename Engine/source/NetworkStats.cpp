#include "NetworkStats.h"
#include "GameObject.h"
#include "Input.h"
#include <iostream>

std::unordered_map<int, int> NetworkStats::clientBytesSent;
std::unordered_map<int, std::vector<unsigned long>> NetworkStats::clientMsgTypeSent;
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
		clientMsgTypeSent[client] = { 0,0,0,0,0,0,0,0 };
		clients.push_back(client);
	}
}

void NetworkStats::registerMsgType(int clientId, int msgType)
{
	clientMsgTypeSent[clientId][msgType] += 1;
}

void NetworkStats::registerBytesSent(int clientID, int numBytes)
{
	clientBytesSent[clientID] += numBytes;
}

void NetworkStats::clearClientBytes()
{
	if (debug) {
		std::cerr << std::endl;
	}
	for (int client : clients) {
		if (debug) {
			std::cerr << "Client " << client << " start__________________________________________" << std::endl;
			std::cerr << "sent " << clientBytesSent[client] << " bytes to client " << client << " this tick" << std::endl;
			std::cerr << "Server also sent client" << client << " these number of messages" << std::endl;
			std::cerr << "CLIENTS_CONN_NETWORK_DATA:   " << clientMsgTypeSent[client][0] << std::endl;
			std::cerr << "CREATE_OBJECT_NETWORK_DATA:  " << clientMsgTypeSent[client][1] << std::endl;
			std::cerr << "DESTROY_OBJECT_NETWORK_DATA: " << clientMsgTypeSent[client][2] << std::endl;
			std::cerr << "INPUT_NETWORK_DATA:          " << clientMsgTypeSent[client][3] << std::endl;
			std::cerr << "TRANSFORM_NETWORK_DATA:      " << clientMsgTypeSent[client][4] << std::endl;
			std::cerr << "MESH_NETWORK_DATA:           " << clientMsgTypeSent[client][5] << std::endl;
			std::cerr << "CAMERA_NETWORK_DATA:         " << clientMsgTypeSent[client][6] << std::endl;
			std::cerr << "LIGHT_NETWORK_DATA:          " << clientMsgTypeSent[client][7] << std::endl;
			std::cerr << "Client " << client << " end___________________________________________" << std::endl;
		}
		clientBytesSent[client] = 0;
	}
	if (debug) {
		std::cerr << std::endl;
	}
}

void NetworkStats::checkDebugButton() {
	debug = Input::getKey("n");
}