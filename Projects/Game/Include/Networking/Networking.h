#pragma once
#include <deque>
#include <memory>
#include <thread>
#include <functional>
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
		
		void AddReplicateProps(const std::vector<ReplicatedProp>& Props);
		void AddRPCProps(const std::vector<RPCProp>& RpcProps);
		void PushRequest(const NetRequest& NewRequest);

#pragma region DELETED METHODS
		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;
#pragma endregion
	
	public:

#ifdef NS_CLIENT // A public client-only functions go here.
		void Client_CallRPC(const RPCSent& RpcRequest);
		void Client_ConnectToServer(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
#endif
#ifdef NS_SERVER // All public server-only functions go here.
		void Server_Listen();
		void Server_RegisterNewActor(Actor* NewActor);
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
#endif
	
	private: // DATA MEMBERS

#ifdef NS_SERVER
		sf::TcpListener ListenerSocket_;
		std::vector<std::unique_ptr<NetClient>> ConnectedClients_;
		sf::SocketSelector Server_Selector_;
		IdentifierType LastActorId = 0;
#endif

#ifdef NS_CLIENT // A private client-only functions go here.
		sf::TcpSocket TCPSocket_;
		sf::SocketSelector Client_Selector_;
#endif
		
		static std::unique_ptr<Networking> Instance_; 
		
		std::deque<NetRequest> IncomingPackets_;
		std::deque<NetRequest> OutgoingPackets_;
		
		std::thread NetworkUpdateThread_;
		bool StopRequested = false;
		
		std::vector<ReplicatedProp> ReplicatedProps_;
		std::unordered_map<Actor*, IdentifierType> ActorRegistry_;
		std::unordered_map<size_t, std::function<void(Actor*)>> FunctionRegistry_;
	};
}
