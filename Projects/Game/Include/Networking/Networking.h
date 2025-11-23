#pragma once
#include <deque>
#include <memory>
#include <thread>
#include <unordered_map>
#include <SFML/Network.hpp>

#include "GameConfiguration.h"

namespace NS
{
	typedef std::uint32_t IdentifierType;
	
	enum class EReliability : uint8_t
	{
		RELIABLE,
		UNRELIABLE
	};

	struct NetClient
	{
		sf::TcpSocket Socket;
		IdentifierType ClientId;
	};

	struct ReplicatedProp
	{
		void* DataPtr;
		size_t Size;
		IdentifierType ActorId;
		IdentifierType ObjectId;
	};

	struct NetPacket
	{
		EReliability Reliability;
		IdentifierType InstanceId;
		ReplicatedProp Property;
		char Data[NS::MAX_PACKET_SIZE];
	};

	struct ReplicationObject
	{
		void* DataPtr = nullptr;
		uint32_t Size;
	};

	void operator<<(sf::Packet& Packet, const NS::ReplicatedProp& Property);
	void operator>>(sf::Packet& Packet, ReplicatedProp& Property);
	void operator<<(sf::Packet& Packet, const NS::NetPacket& Request);
	void operator>>(sf::Packet& Packet, NS::NetPacket& Request);

	class Networking
	{
		friend class Engine;
		struct ReplicatedMemAttrib
		{
			void* SourcePtr;
			void* DestinationPtr;
			size_t SizeInBytes;
		};

	public:

		[[nodiscard]] static Networking* Get();

#pragma region DELETED METHODS
		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;
#pragma endregion
	
	public:

#ifdef NS_CLIENT // A public client-only functions go here.
		void Client_ConnectToServer(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
		void Client_ReplicateFromServer(void* Data, uint16_t Size, uint32_t ObjectId);
#endif
#ifdef NS_SERVER // All public server-only functions go here.
		void Server_Listen();
		void Server_ReplicateToClient(const ReplicatedProp& Property, IdentifierType InstanceId);
#endif

	private:
		Networking() = default;
		void Start();
		void Stop();
		void PushRequest(const NetPacket& NewRequest);
		void ProcessRequests();
		void AddReplicateProps(const std::vector<ReplicatedProp>& Props);
		ReplicatedMemAttrib Unmap(const ReplicatedProp& Property);

#ifdef NS_SERVER // All private server-only functions go here.
		void Server_SendPackets();
		void Server_ReceivePackets();
		void Server_ProcessRequest(const NetPacket& Request);
#endif

#ifdef NS_CLIENT // All private client-only functions go here.
		void Client_SendPackets();
		void Client_ReceivePackets();
		void Client_ProcessRequest(NS::NetPacket Request);
#endif
	
	private: // DATA MEMBERS

#ifdef NS_SERVER
		sf::TcpListener ListenerSocket_;
		std::vector<std::unique_ptr<NetClient>> ConnectedClients_;
		sf::SocketSelector Server_Selector_;
#endif

#ifdef NS_CLIENT // A private client-only functions go here.
		sf::TcpSocket TCPSocket_;
		sf::SocketSelector Client_Selector_;
#endif
		
		static std::unique_ptr<Networking> Instance_;
		
		std::deque<NetPacket> IncomingRequests_;
		std::deque<NetPacket> OutgoingRequests_;
		std::unordered_map<uint32_t, ReplicationObject> ReplicationMap_;
		
		std::thread NetworkUpdateThread_;
		bool StopRequested = false;
		
		std::vector<ReplicatedProp> ReplicatedProps_;
	};
}
