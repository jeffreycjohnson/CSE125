#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "ForwardDecs.h"
#include "NetworkUtility.h"
#include "NetworkStruct.h"

#include <map>
#include <utility>
#include <vector>
#include <tuple>

typedef int ClientID;

enum NetworkState
{
	UNINITIALIZED,
	INITIALIZING,
	SHUTDOWN,
	CLIENT_MODE,
	SERVER_MODE,
	OFFLINE
};

class NetworkManager
{
private:
	static NetworkState state;

	static std::vector<ClientID> clientIDs;
	static ClientID myClientID;

	static std::map<std::pair<int, int>, NetworkResponse> postbox;

	static void ReceiveServerMessages();
	static void SendServerMessages();

	static void ReceiveClientMessages();
	static void SendClientMessages();
public:
	NetworkManager();
	~NetworkManager();

	static std::vector<ClientID> InitializeServer(std::string port, int numberOfClients);
	static std::tuple<std::vector<ClientID>, ClientID> InitializeClient(std::string serverIP, std::string port);
	static void InitializeOffline();

	static void PostMessage(const std::vector<char>& bytes, int messageType, int messageID);

	static NetworkState getState();
};

#endif // NETWORK_MANAGER_H