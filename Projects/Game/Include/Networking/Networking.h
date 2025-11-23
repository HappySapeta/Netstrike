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
		class Actor* ActorPtr;
		size_t Offset;
		size_t Size;
	};

	struct NetPacket
	{
		EReliability Reliability;
		IdentifierType InstanceId;
		IdentifierType ActorId;
		size_t ObjectOffset;
		char Data[NS::MAX_PACKET_SIZE];
	};

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
#endif
#ifdef NS_SERVER // All public server-only functions go here.
		void Server_Listen();
#endif

	private:
		Networking() = default;
		void Start();
		void Stop();
		void PushRequest(const NetPacket& NewRequest);
		void UpdateThread();
		void UpdateReplicated();
		void AddReplicateProps(const std::vector<ReplicatedProp>& Props);

#ifdef NS_SERVER // All private server-only functions go here.
		void Server_SendPackets();
		void Server_ReceivePackets();
#endif

#ifdef NS_CLIENT // All private client-only functions go here.
		void Client_SendPackets();
		void Client_ReceivePackets();
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
		
		std::deque<NetPacket> IncomingPackets_;
		std::deque<NetPacket> OutgoingPackets_;
		
		std::thread NetworkUpdateThread_;
		bool StopRequested = false;
		
		std::vector<ReplicatedProp> ReplicatedProps_;
		std::unordered_map<IdentifierType, ReplicatedProp> ReplicationMap_;
		std::unordered_map<Actor*, IdentifierType> ActorRegistry_;
	};
}
