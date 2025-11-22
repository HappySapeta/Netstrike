#pragma once
#include <deque>
#include <memory>
#include <thread>
#include <unordered_map>
#include <SFML/Network.hpp>

#include "GameConfiguration.h"
#include "Logger.h"

namespace NS
{
	typedef uint32_t IdentifierType;
	
	enum class EReliability : uint8_t
	{
		RELIABLE,
		UNRELIABLE
	};

	enum class ENetAuthority : uint8_t
	{
		SERVER,
		CLIENT
	};

	struct NetClient
	{
		sf::TcpSocket Socket;
		IdentifierType ClientId;
	};
	
	struct NetPacket
	{
		EReliability Reliability;
		IdentifierType InstanceId;
		IdentifierType ObjectId;
		size_t Size;
		char Data[NS::PACKET_SIZE];
	};
	
	struct ReplicatedProp
	{
		void* Ptr;
		size_t Size;
		IdentifierType ActorId;
		IdentifierType ObjectId;
	};

	struct ReplicationObject
	{
		void* DataPtr = nullptr;
		uint32_t Size;
	};

	inline void operator<<(sf::Packet& Packet, const NS::NetPacket& Request)
	{
		Packet << static_cast<std::underlying_type_t<EReliability>>(Request.Reliability);
		Packet << Request.InstanceId;
		Packet << Request.ObjectId;
		Packet << Request.Size;
		
		Packet.append(Request.Data, sizeof(Request.Data));
	}

	inline void operator>>(sf::Packet& Packet, NS::NetPacket& Request)
	{
		uint8_t Byte;
		
		Packet >> Byte;
		Request.Reliability = static_cast<NS::EReliability>(Byte);
	
		Packet >> Request.InstanceId;
		Packet >> Request.ObjectId;
		Packet >> Request.Size;
		
		memcpy(Request.Data, static_cast<const char*>(Packet.getData()) + Packet.getReadPosition(), sizeof(Request.Data));
	}
	
	class Networking
	{
		friend class Engine;
	public:

		[[nodiscard]] static Networking* Get()
		{
			if (!Instance_)
			{
				Instance_ = std::unique_ptr<Networking>(new Networking());
			}

			return Instance_.get();
		}

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
		void Server_ReplicateToClient(const void* Data, uint16_t Size, uint32_t ObjectId, uint8_t ClientId);
		
#endif

	private:

		Networking();
		void Start();
		void Stop();
		void PushRequest(const NetPacket& NewRequest);
		void ProcessRequests();
		void AddReplicateProps(const std::vector<ReplicatedProp>& Props);
		NS::ReplicationObject Unmap(uint32_t ObjectId);

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
		
		const ENetAuthority NetIdentity_;

		std::deque<NetPacket> IncomingRequests_;
		std::deque<NetPacket> OutgoingRequests_;
		std::unordered_map<uint32_t, ReplicationObject> ReplObjectMap_;
		
		std::thread NetworkUpdateThread_;
		bool StopRequested = false;
		
		std::vector<ReplicatedProp> ReplicatedProps_;
	};
}
