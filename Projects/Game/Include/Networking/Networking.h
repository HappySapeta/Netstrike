#pragma once
#include <deque>
#include <memory>
#include <unordered_map>
#include <SFML/Network.hpp>

#include "GameConfiguration.h"

namespace NS
{
	enum EReliability : uint8_t
	{
		RELIABLE,
		UNRELIABLE
	};

	enum EInstructionType : uint8_t
	{
		RPC,
		REPLICATION
	};

	enum ENetAuthority : uint8_t
	{
		SERVER,
		CLIENT
	};

	struct NetRequest
	{
		EReliability Reliability;
		EInstructionType InstructionType;
		ENetAuthority NetSource;
		uint8_t InstanceId;
		
		uint32_t ObjectId;
		uint64_t Size;
		char Data[NS::PACKET_SIZE];
	};

	struct ReplicationObject
	{
		void* DataPtr = nullptr;
		uint64_t Size;
	};

	inline void operator<<(sf::Packet& Packet, const NS::NetRequest& Request)
	{
		Packet << Request.InstructionType;
		Packet << Request.NetSource;
		Packet << Request.InstanceId;
		Packet << Request.ObjectId;
		Packet << Request.Size;
		Packet << Request.Data;
	}

	inline void operator>>(sf::Packet& Packet, NS::NetRequest& Request)
	{
		uint8_t Byte;
	
		Packet >> Byte;
		Request.InstructionType = static_cast<NS::EInstructionType>(Byte);

		Packet >> Byte;
		Request.NetSource = static_cast<NS::ENetAuthority>(Byte);
	
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

	public:

#ifdef NS_CLIENT
		[[nodiscard]] sf::TcpSocket& TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
#endif

#ifdef NS_SERVER
		[[nodiscard]] sf::TcpListener& TCPListen();
		std::vector<sf::TcpSocket>& GetClientSockets()
		{
			return ConnectedClientSockets_;
		}
#endif

	public:

		void PushRequest(const NetRequest& NewRequest);

	private:

		Networking();
		void ProcessRequests();
		NS::ReplicationObject Unmap(uint32_t ObjectId);

	private:

		const ENetAuthority NetIdentity_;
		static std::unique_ptr<Networking> Instance_;
		
#ifdef NS_CLIENT
		sf::TcpSocket ServerSocket_;
#endif
#ifdef NS_SERVER
		sf::TcpListener ListenerSocket_;
		std::vector<sf::TcpSocket> ConnectedClientSockets_;
#endif

		std::deque<NetRequest> IncomingRequests_;
		std::deque<NetRequest> OutgoingRequests_;
		std::unordered_map<uint32_t, ReplicationObject> ReplObjectMap_;
	};
}
