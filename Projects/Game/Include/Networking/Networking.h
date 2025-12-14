#pragma once
#include <deque>
#include <memory>
#include <thread>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "Networking-Types.h"

namespace NS
{
	class Networking
	{
	public:

		[[nodiscard]] static Networking* Get();
		void Start();
		void Stop();
		void Update();

		void AddReplicateProps(const std::vector<ReplicatedProp>& Props);
		void AddRPCProps(const std::vector<RPCProp>& RpcProps);
		void PushRequest(const NetRequest& NewRequest);
		bool HasStarted();

#pragma region DELETED METHODS
		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;
#pragma endregion
	
	public:

#ifdef NS_CLIENT // A public client-only functions go here.
		bool IsConnectedToServer() const
		{
			return bIsConnectedToServer;
		}
		IdentifierType Client_GetNetId() const
		{
			return NetId_;
		}
		void Client_CallRPC(const RPCSent& RpcRequest);
		void Client_ConnectToServer(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
#endif
#ifdef NS_SERVER // All public server-only functions go here.
		void Server_SetMaxConnections(const int NumMaxConnections);
		bool Server_HasConnections() const
		{
			return !ConnectedClients_.empty();
		}
		void Server_AssignOnClientConnected(OnClientConnectedDelegate Callback);
		void Server_CallRPC(const RPCSent& RpcRequest, const Actor* Player = nullptr);
		void Server_Listen();
		void Server_RegisterNewActor(Actor* NewActor, IdentifierType AuthNetId = -1);
		void Server_DeRegisterActor(Actor* Actor);
#endif

	private:
		Networking() = default;
		void UpdateThread();
		void ProcessRequest_RPCReceived(const RPCReceived& RpcRequest);
		sf::Socket::Status SendPacketHelper(sf::Packet& Packet, sf::TcpSocket& Socket);

#ifdef NS_SERVER // All private server-only functions go here.
		void Server_SendPackets();
		void Server_ReceivePackets();
		void Server_ProcessRequests();
#endif

#ifdef NS_CLIENT // All private client-only functions go here.
		void Client_SendPackets();
		void Client_ReceivePackets();
		void Client_ProcessRequests();
		void Client_ProcessRequest_Replication(const NetRequest& Request);
		void Client_ProcessRequest_ActorCreate(const NetRequest& Request);
		void Client_ProcessRequest_RPC(const NetRequest& Packet);
		void Client_ProcessRequest_IDAssignment(const NetRequest& Packet);
		void Client_ProcessRequest_ActorDestruction(const NetRequest& Request);
#endif
	
	private: // DATA MEMBERS

#ifdef NS_SERVER
		sf::TcpListener ListenerSocket_;
		ClientVectorType ConnectedClients_;
		sf::SocketSelector Server_Selector_;
		IdentifierType LastActorId = 0;
		OnClientConnectedDelegate OnClientConnected;
		int NumMaxConnections_;
#endif

#ifdef NS_CLIENT // A private client-only functions go here.
		bool bIsConnectedToServer = false;
		sf::TcpSocket TCPSocket_;
		sf::SocketSelector Client_Selector_;
		IdentifierType NetId_ = -1;
#endif
		
		static std::unique_ptr<Networking> Instance_; 
		
		std::deque<NetRequest> IncomingPackets_;
		std::deque<NetRequest> OutgoingPackets_;
		
		std::jthread NetworkUpdateThread_;
		bool StopRequested = false;
		bool hasStarted = false;
		
		std::vector<ReplicatedProp> ReplicatedProps_;
		std::unordered_map<Actor*, IdentifierType> ActorRegistry_;
		std::unordered_map<size_t, std::function<void(Actor*)>> FunctionRegistry_;
		
		std::mutex QueueMutex_;
	};
}
