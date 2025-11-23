#include <functional>

#include "Networking/Networking.h"
#include "GameConfiguration.h"
#include "Logger.h"
#include "Actor/Actor.h"
#include "Engine/Engine.h"

std::unique_ptr<NS::Networking> NS::Networking::Instance_(nullptr);

NS::Networking* NS::Networking::Get()
{
	if (!Instance_)
	{
		Instance_ = std::unique_ptr<Networking>(new Networking());
	}

	return Instance_.get();
}

void NS::operator<<(sf::Packet& Packet, const NS::NetPacket& Request)
{
	Packet << static_cast<std::underlying_type_t<EReliability>>(Request.Reliability);
	Packet << Request.InstanceId;
	Packet << Request.ActorId;
	Packet << Request.ObjectOffset;
	Packet.append(Request.Data, sizeof(Request.Data));
}

void NS::operator>>(sf::Packet& Packet, NS::NetPacket& Request)
{
	uint8_t Byte;
	Packet >> Byte;
	Request.Reliability = static_cast<NS::EReliability>(Byte);
	
	Packet >> Request.InstanceId;
	Packet >> Request.ActorId;
	Packet >> Request.ObjectOffset;
	
	memcpy(Request.Data, static_cast<const char*>(Packet.getData()) + Packet.getReadPosition(), sizeof(Request.Data));
}

void NS::Networking::PushRequest(const NetPacket& NewRequest)
{
	OutgoingPackets_.push_back(NewRequest);
}

void NS::Networking::Start()
{
	NSLOG(ELogLevel::INFO, "Starting network update thread.");
	NetworkUpdateThread_ = std::thread(std::bind(&NS::Networking::UpdateThread, this));
}

void NS::Networking::Stop()
{
	StopRequested = true;
	if (NetworkUpdateThread_.joinable())
	{
		NSLOG(ELogLevel::INFO, "Stopping network update thread.");
		NetworkUpdateThread_.join();
	}
	else
	{
		NSLOG(ELogLevel::INFO, "Failed to join network update thread.");
	}
}

void NS::Networking::AddReplicateProps(const std::vector<ReplicatedProp>& Props)
{
	for (const ReplicatedProp& Prop : Props)
	{
		if (ActorRegistry_.contains(Prop.ActorPtr))
		{
			const IdentifierType ActorId = ActorRegistry_.at(Prop.ActorPtr); // TODO : Fill actor registry.
			ReplicationMap_[ActorId] = Prop;
			ReplicatedProps_.push_back(Prop);
		}
	}
}

void NS::Networking::UpdateThread()
{
	while (!StopRequested)
	{
		// Populate the queue with incoming requests
#ifdef NS_CLIENT
		Client_SendPackets();
		Client_ReceivePackets();
#endif

#ifdef NS_SERVER
		Server_SendPackets();
		Server_ReceivePackets();
#endif
		
		ProcessRequests();
	}
}

void NS::Networking::ProcessRequests()
{
#ifdef NS_CLIENT
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
#endif
#ifdef NS_SERVER
	for (const ReplicatedProp& Prop : ReplicatedProps_)
	{
		if (!ActorRegistry_.contains(Prop.ActorPtr))
		{
			continue;
		}
		
		const IdentifierType ActorId = ActorRegistry_.at(Prop.ActorPtr);
		NetPacket ReplicationRequest;
		ReplicationRequest.Reliability = EReliability::RELIABLE;
		ReplicationRequest.InstanceId = 0; // TODO : Handle multiple clients
		ReplicationRequest.ActorId = ActorId;
		ReplicationRequest.ObjectOffset = Prop.Offset;
		
		void* DataPtr = Prop.ActorPtr + Prop.Offset;
		memcpy_s(ReplicationRequest.Data, NS::MAX_PACKET_SIZE, DataPtr, Prop.Size);
			
		PushRequest(ReplicationRequest);
	}
	
#endif
}
