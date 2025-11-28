#include <functional>

#include "Networking/Networking.h"
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
	Packet << static_cast<std::underlying_type_t<ERequestType>>(Request.RequestType);
	Packet << Request.InstanceId;
	Packet << Request.ActorId;
	Packet << Request.ObjectOffset;
	Packet << Request.DataSize;
	Packet.append(Request.Data, sizeof(Request.Data));
}

void NS::operator>>(sf::Packet& Packet, NS::NetPacket& Request)
{
	std::underlying_type_t<EReliability> ReliabilityData;
	Packet >> ReliabilityData;
	Request.Reliability = static_cast<NS::EReliability>(ReliabilityData);
	
	std::underlying_type_t<ERequestType> RequestTypeData;
	Packet >> RequestTypeData;
	Request.RequestType = static_cast<NS::ERequestType>(RequestTypeData);
	
	Packet >> Request.InstanceId;
	Packet >> Request.ActorId;
	Packet >> Request.ObjectOffset;
	Packet >> Request.DataSize;
	
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
		Client_ProcessRequests();
#endif

#ifdef NS_SERVER
		Server_SendPackets();
		Server_ReceivePackets();
		Server_ProcessRequests();
#endif
	}
}
