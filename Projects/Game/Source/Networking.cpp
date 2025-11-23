#include <functional>

#include "Networking/Networking.h"
#include "GameConfiguration.h"
#include "Logger.h"

std::unique_ptr<NS::Networking> NS::Networking::Instance_(nullptr);

NS::Networking* NS::Networking::Get()
{
	if (!Instance_)
	{
		Instance_ = std::unique_ptr<Networking>(new Networking());
	}

	return Instance_.get();
}

void NS::operator<<(sf::Packet& Packet, const NS::ReplicatedProp& Property)
{
	Packet << Property.ActorId;
	Packet << Property.ObjectId;
	Packet << Property.Size;
}

void NS::operator>>(sf::Packet& Packet, ReplicatedProp& Property)
{
	Packet >> Property.ActorId;
	Packet >> Property.ObjectId;
	Packet >> Property.Size;
}

void NS::operator<<(sf::Packet& Packet, const NS::NetPacket& Request)
{
	Packet << static_cast<std::underlying_type_t<EReliability>>(Request.Reliability);
	Packet << Request.InstanceId;
	Packet << Request.Property;
	Packet.append(Request.Data, sizeof(Request.Data));
}

void NS::operator>>(sf::Packet& Packet, NS::NetPacket& Request)
{
	uint8_t Byte;
	Packet >> Byte;
	Request.Reliability = static_cast<NS::EReliability>(Byte);
	Packet >> Request.InstanceId;
	Packet >> Request.Property;
	memcpy(Request.Data, static_cast<const char*>(Packet.getData()) + Packet.getReadPosition(), sizeof(Request.Data));
}

void NS::Networking::PushRequest(const NetPacket& NewRequest)
{
	OutgoingRequests_.push_back(NewRequest);
}

void NS::Networking::Start()
{
	NSLOG(ELogLevel::INFO, "Starting network update thread.");
	NetworkUpdateThread_ = std::thread(std::bind(&NS::Networking::ProcessRequests, this));
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
	ReplicatedProps_.insert(ReplicatedProps_.end(), Props.begin(), Props.end());
	for (const ReplicatedProp& Prop : Props)
	{
	}
}

void NS::Networking::ProcessRequests()
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
	}
}

NS::Networking::ReplicatedMemAttrib NS::Networking::Unmap(const ReplicatedProp& Property)
{
	return {};
}
