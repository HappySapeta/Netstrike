#pragma once
#include <deque>
#include <memory>
#include <thread>
#include <unordered_map>

#include "Networking-Types.h"

namespace NS
{
	class Networking
	{
		struct ReplicatedMemAttrib
		{
			void* SourcePtr;
			void* DestinationPtr;
			size_t SizeInBytes;
		};

	public:

		[[nodiscard]] static Networking* Get();
		void Start();
		void Stop();
		
		void AddReplicateProps(const std::vector<ReplicatedProp>& Props);
		void PushRequest(const NetPacket& NewRequest);

#pragma region DELETED METHODS
		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;
#pragma endregion
	
	public:

#ifdef NS_CLIENT // A public client-only functions go here.
		void Client_ConnectToServer(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
#endif
#ifdef NS_SERVER // All public server-only functions go here.
		void Server_Listen();
		void Server_RegisterNewActor(Actor* NewActor);
#endif

	private:
		Networking() = default;
		void UpdateThread();

#ifdef NS_SERVER // All private server-only functions go here.
		void Server_SendPackets();
		void Server_ReceivePackets();
		void Server_ProcessRequests();
#endif

#ifdef NS_CLIENT // All private client-only functions go here.
		void Client_SendPackets();
		void Client_ReceivePackets();
		void Client_ProcessRequests();
		void Client_ProcessRequest_Replication(const NetPacket& Packet);
		void Client_ProcessRequest_ActorCreate(const NetPacket& Packet);
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
		
		std::deque<NetPacket> IncomingPackets_;
		std::deque<NetPacket> OutgoingPackets_;
		
		std::thread NetworkUpdateThread_;
		bool StopRequested = false;
		
		std::vector<ReplicatedProp> ReplicatedProps_;
		std::unordered_map<IdentifierType, ReplicatedProp> ReplicationMap_;
		std::unordered_map<Actor*, IdentifierType> ActorRegistry_;
	};
}
