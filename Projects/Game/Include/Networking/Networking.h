#pragma once
#include <deque>
#include <memory>
#include <unordered_map>
#include <SFML/Network.hpp>

#include "GameConfiguration.h"

namespace NS
{
	enum class EReliability : uint8_t
	{
		RELIABLE,
		UNRELIABLE
	};

	enum class EInstructionType : uint8_t
	{
		RPC,
		REPLICATION
	};

	enum class ENetAuthority : uint8_t
	{
		SERVER,
		CLIENT
	};

	struct NetRequest
	{
		EReliability Reliability;
		EInstructionType InstructionType;
		uint8_t InstanceId;
		uint32_t ObjectId;
		uint64_t Size;
		char Data[NS::PACKET_SIZE];
	};

	struct ReplicationObject
	{
		void* DataPtr = nullptr;
		uint32_t Size;
	};

	inline void operator<<(sf::Packet& Packet, const NS::NetRequest& Request)
	{
		Packet << static_cast<std::underlying_type_t<EReliability>>(Request.Reliability);
		Packet << static_cast<std::underlying_type_t<EInstructionType>>(Request.InstructionType);
		Packet << Request.InstanceId;
		Packet << Request.ObjectId;
		Packet << Request.Size;
		Packet << Request.Data;
	}

	inline void operator>>(sf::Packet& Packet, NS::NetRequest& Request)
	{
		uint8_t Byte;
		
		Packet >> Byte;
		Request.Reliability = static_cast<NS::EReliability>(Byte);
	
		Packet >> Byte;
		Request.InstructionType = static_cast<NS::EInstructionType>(Byte);
	
		Packet >> Request.InstanceId;
		Packet >> Request.ObjectId;
		Packet >> Request.Size;
		Packet >> Request.Data;
	}
	
	class Networking
	{
	public:

		[[nodiscard]] static Networking* Get()
		{
			if (!Instance_)
			{
				Instance_ = std::unique_ptr<Networking>(new Networking());
			}

			return Instance_.get();
		}

		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;
		
		void PushRequest(const NetRequest& NewRequest);
		void Update();

	public:

#ifdef NS_CLIENT // A public client-only functions go here.
		[[nodiscard]] sf::TcpSocket& TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
#endif

#ifdef NS_SERVER // All public server-only functions go here.
		[[nodiscard]] sf::TcpListener& Server_Listen();
		std::vector<sf::TcpSocket>& GetClientSockets()
		{
			return ConnectedClientSockets_;
		}
#endif

	private:

		Networking();
		void ProcessRequests();
		NS::ReplicationObject Unmap(uint32_t ObjectId);

#ifdef NS_SERVER // All private server-only functions go here.
		void Server_SendPackets();
		void Server_ReceivePackets();
		void Server_ProcessRequest(const NetRequest& Request);
#endif

#ifdef NS_CLIENT // A private client-only functions go here.
		void Client_SendPackets();
		void Client_ReceivePackets();
		void Client_ProcessRequest(NS::NetRequest Request);
#endif
		
	private: // DATA MEMBERS

#ifdef NS_SERVER
		sf::TcpListener ListenerSocket_;
		std::vector<sf::TcpSocket> ConnectedClientSockets_;
#endif

#ifdef NS_CLIENT // A private client-only functions go here.
		sf::TcpSocket TCPSocket_;
#endif
		
		static std::unique_ptr<Networking> Instance_;
		const ENetAuthority NetIdentity_;

		std::deque<NetRequest> IncomingRequests_;
		std::deque<NetRequest> OutgoingRequests_;
		std::unordered_map<uint32_t, ReplicationObject> ReplObjectMap_;
	};
}
