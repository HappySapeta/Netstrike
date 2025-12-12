#ifdef NS_CLIENT

#include "Logger.h"
#include "Engine/Engine.h"
#include "Networking/Networking.h"

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
				{
					std::lock_guard<std::mutex> QueueLock(QueueMutex_);
					IncomingPackets_.emplace_back(Request);
				}
			}
		}
	}
}

void NS::Networking::Client_ProcessRequests()
{
	std::lock_guard<std::mutex> QueueLock(QueueMutex_);
	while (!IncomingPackets_.empty())
	{
		NetRequest Request = IncomingPackets_.front();
		IncomingPackets_.pop_front();

		switch (Request.RequestType)
		{
			case ERequestType::ACTOR_CREATION:
			{
				Client_ProcessRequest_ActorCreate(Request);
				break;
			}
			case ERequestType::REPLICATION:
			{
				Client_ProcessRequest_Replication(Request);
				break;
			}
			case ERequestType::RPC:
			{
				Client_ProcessRequest_RPC(Request);
				break;
			}
			case ERequestType::ID_ASSIGNMENT:
			{
				Client_ProcessRequest_IDAssignment(Request);
				break;
			}
		}
	}
}

void NS::Networking::Client_CallRPC(const RPCSent& RpcRequest)
{
	NetRequest Request;
	Request.Reliability = EReliability::RELIABLE;
	Request.RequestType = ERequestType::RPC;
	Request.InstanceId = 0;
	Request.ActorId = ActorRegistry_.at(RpcRequest.Actor);
	Request.ObjectOffset = 0;
	Request.DataSize = sizeof(size_t);
	
	std::hash<std::string> Hasher;
	size_t FunctionHash = Hasher(RpcRequest.FunctionName);
	
	memcpy_s(Request.Data, NS::MAX_PACKET_SIZE, &FunctionHash, sizeof(size_t)); // TODO : Use user defined type for hash.
	
	OutgoingPackets_.push_back(Request);
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
	
	if (Request.InstanceId == NetId_)
	{
		NSLOG(ELogLevel::INFO, "Actor created for me! {}", Request.InstanceId);
	}
	
	Actor* NewActor = Engine::Get()->CreateActor(TypeHash);
	NewActor->SetNetId(NetId_);
	ActorRegistry_[NewActor] = Request.ActorId;
}

void NS::Networking::Client_ProcessRequest_RPC(const NetRequest& Packet)
{
	RPCReceived RpcReceived;
	RpcReceived.ActorId = Packet.ActorId;
	memcpy_s(&RpcReceived.FunctionHash, sizeof(size_t), Packet.Data, Packet.DataSize);
	ProcessRequest_RPCReceived(RpcReceived);
}

void NS::Networking::Client_ProcessRequest_IDAssignment(const NetRequest& Packet)
{
	NetId_ = Packet.InstanceId;
	NSLOG(ELogLevel::INFO, "NetId assigned : {}", NetId_);
}

#endif
