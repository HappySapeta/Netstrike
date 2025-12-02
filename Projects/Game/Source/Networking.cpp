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

void NS::operator<<(sf::Packet& Packet, const NS::NetRequest& Request)
{
	Packet << static_cast<std::underlying_type_t<EReliability>>(Request.Reliability);
	Packet << static_cast<std::underlying_type_t<ERequestType>>(Request.RequestType);
	Packet << Request.InstanceId;
	Packet << Request.ActorId;
	Packet << Request.ObjectOffset;
	Packet << Request.DataSize;
	Packet.append(Request.Data, sizeof(Request.Data));
}

void NS::operator>>(sf::Packet& Packet, NS::NetRequest& Request)
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

void NS::Networking::PushRequest(const NetRequest& NewRequest)
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

void NS::Networking::AddRPCProps(const std::vector<RPCProp>& RpcProps)
{
	std::hash<std::string> Hasher;
	for (const RPCProp& Prop : RpcProps)
	{
		const size_t Hash = Hasher(Prop.FunctionName);
		FunctionRegistry_[Hash] = Prop.Callback;
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

void NS::Networking::ProcessRequest_RPCReceived(const RPCReceived& RpcReceived)
{
	Actor* Actor = nullptr;
	for (const auto& [Ptr, Id] : ActorRegistry_)
	{
		if (Id == RpcReceived.ActorId)
		{
			Actor = Ptr;
			break;
		}
	}
	
	const auto RPC = FunctionRegistry_.at(RpcReceived.FunctionHash);
	std::invoke(RPC, Actor);
}

sf::Socket::Status NS::Networking::SendPacketHelper(sf::Packet& Packet, sf::TcpSocket& Socket)
{
	const sf::Socket::Status& SendStatus = Socket.send(Packet);
	if (SendStatus == sf::Socket::Status::Error)
	{
		NSLOG(ELogLevel::ERROR, "Failed to send packet. {}:{}", Socket.getRemoteAddress()->toString(), Socket.getRemotePort());
	}
	
	return SendStatus;
}
