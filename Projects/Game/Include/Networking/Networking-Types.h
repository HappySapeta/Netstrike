#pragma once
#include <cstdint>
#include <SFML/Network.hpp>

#include "GameConfiguration.h"


namespace NS
{
	typedef std::uint64_t IdentifierType;
	typedef int InstanceIdType;
	static constexpr InstanceIdType NS_BROADCAST_ID = -1;

	enum class EReliability : uint8_t
	{
		RELIABLE,
		UNRELIABLE
	};
	
	enum class ERequestType : uint8_t
	{
		REPLICATION,
		RPC,
		ACTOR_CREATION
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
		ERequestType RequestType;
		InstanceIdType InstanceId;
		IdentifierType ActorId;
		size_t ObjectOffset;
		char Data[NS::MAX_PACKET_SIZE];
	};

	void operator<<(sf::Packet& Packet, const NS::NetPacket& Request);
	void operator>>(sf::Packet & Packet, NS::NetPacket & Request);
}
