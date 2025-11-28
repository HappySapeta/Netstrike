#ifdef NS_CLIENT

#include "Logger.h"
#include "Engine/Engine.h"
#include "Networking/Networking.h"

// TODO: Use Non-blocking sockets if possible.
void NS::Networking::Client_ConnectToServer(const sf::IpAddress& ServerAddress, const uint16_t ServerPort)
{
	TCPSocket_.disconnect();
	const auto ConnectStatus = TCPSocket_.connect(ServerAddress, ServerPort, sf::seconds(NS::DEFAULT_CONNECTION_TIMEOUT));
	if (ConnectStatus != sf::Socket::Status::Done)
	{
		NSLOG(NS::ELogLevel::ERROR, "Failed to connect to Server with address {}:{}", ServerAddress.toString(), ServerPort);
	}
	else
	{
		NSLOG(NS::ELogLevel::INFO, "Connected successfully to server at {}:{}", ServerAddress.toString(), ServerPort);
	}

	TCPSocket_.setBlocking(false);
	Client_Selector_.add(TCPSocket_);
}

void NS::Networking::Client_SendPackets()
{
	while (!OutgoingPackets_.empty())
	{
		NS::NetPacket Request = OutgoingPackets_.front();
		OutgoingPackets_.pop_front();
		
		sf::Packet Packet;
		Packet << Request;
		
		if (Request.Reliability == EReliability::RELIABLE)
		{
			const auto SendStatus = TCPSocket_.send(Packet);
			if (SendStatus != sf::Socket::Status::Done)
			{
				NSLOG(ELogLevel::ERROR, "[CLIENT] Failed to send packet to server!");
			}
		}
	}
}

void NS::Networking::Client_ReceivePackets()
{
	if (Client_Selector_.wait(sf::milliseconds(NS::CLIENT_SELECTOR_WAIT_TIME_MS)))
	{
		if (Client_Selector_.isReady(TCPSocket_))
		{
			sf::Packet Packet;
			const auto ReceiveStatus = TCPSocket_.receive(Packet);
			if (ReceiveStatus == sf::Socket::Status::Error)
			{
				NSLOG(LOGERROR, "[CLIENT] Failed to receive packet.");
			}
			else if (ReceiveStatus == sf::Socket::Status::Done)
			{
				NS::NetPacket Request;
				Packet >> Request;
				IncomingPackets_.emplace_back(Request);
			}
		}
	}
}

void NS::Networking::Client_ProcessRequests()
{
	while (!IncomingPackets_.empty())
	{
		NetPacket Packet = IncomingPackets_.front();
		IncomingPackets_.pop_front();

		switch (Packet.RequestType)
		{
			case ERequestType::ACTOR_CREATION:
			{
				Client_ProcessRequest_ActorCreate(Packet);
				break;
			}
		}
	}
}

void NS::Networking::Client_ProcessRequest_ActorCreate(const NetPacket& Packet)
{
	size_t TypeHash;
	memcpy_s(&TypeHash, sizeof(TypeHash), Packet.Data, Packet.DataSize);
	
	Actor* NewActor = Engine::Get()->CreateActor(TypeHash);
	ActorRegistry_[NewActor] = Packet.ActorId;
}

#endif
