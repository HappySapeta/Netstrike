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
		NS::NetRequest Request = OutgoingPackets_.front();
		OutgoingPackets_.pop_front();
		
		sf::Packet Packet;
		Packet << Request;
		
		if (Request.Reliability == EReliability::RELIABLE)
		{
			SendPacketHelper(Packet, TCPSocket_);
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
				NS::NetRequest Request;
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
		NetRequest Packet = IncomingPackets_.front();
		IncomingPackets_.pop_front();

		switch (Packet.RequestType)
		{
			case ERequestType::ACTOR_CREATION:
			{
				Client_ProcessRequest_ActorCreate(Packet);
				break;
			}
			case ERequestType::REPLICATION:
			{
				Client_ProcessRequest_Replication(Packet);
				break;
			}
		}
	}
}

void NS::Networking::Client_ProcessRequest_Replication(const NetRequest& Request)
{
	Actor* ActorPtr = nullptr;
	for (const auto& [Ptr, ActorId] : ActorRegistry_)
	{
		if (ActorId == Request.ActorId)
		{
			ActorPtr = Ptr;
		}
	}
	
	void* DataPtr = reinterpret_cast<char*>(ActorPtr) + Request.ObjectOffset;
	memcpy_s(DataPtr, Request.DataSize, Request.Data, Request.DataSize);
}

void NS::Networking::Client_ProcessRequest_ActorCreate(const NetRequest& Request)
{
	size_t TypeHash;
	memcpy_s(&TypeHash, sizeof(TypeHash), Request.Data, Request.DataSize);
	
	Actor* NewActor = Engine::Get()->CreateActor(TypeHash);
	ActorRegistry_[NewActor] = Request.ActorId;
}

#endif
