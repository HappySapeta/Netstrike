#ifdef NS_SERVER

#include "Logger.h"
#include "Actor/Actor.h"
#include "Networking/Networking.h"

void NS::Networking::Server_CallRPC(const RPCSent& RpcRequest, const Actor* Player)
{
	NetRequest Request;
	Request.Reliability = EReliability::RELIABLE;
	Request.RequestType = ERequestType::RPC;
	Request.InstanceId = -1;
	Request.ActorId = ActorRegistry_.at(RpcRequest.Actor);
	Request.ObjectOffset = 0;
	Request.DataSize = sizeof(size_t);
	
	std::hash<std::string> Hasher;
	size_t FunctionHash = Hasher(RpcRequest.FunctionName);
	
	memcpy_s(Request.Data, NS::MAX_PACKET_SIZE, &FunctionHash, sizeof(size_t)); // TODO : Use user defined type for hash.
	
	PushRequest(Request);
}

void NS::Networking::Server_Listen()
{
	ListenerSocket_.close();
	NSLOG(NS::ELogLevel::INFO, "Listening for {} connections on port {}", NumMaxConnections_, NS::SERVER_PORT);
	const sf::Socket::Status ListenStatus = ListenerSocket_.listen(NS::SERVER_PORT);
	
	if (ListenStatus != sf::Socket::Status::Done)
	{
		NSLOG(ELogLevel::ERROR, "[SERVER] Failed to listen on port {}.", NS::SERVER_PORT);
		return;
	}

	int NumClients = NumMaxConnections_;
	while (NumClients > 0)
	{
		ConnectedClients_.emplace_back(std::make_unique<NetClient>());
		std::unique_ptr<NetClient>& NewClient = ConnectedClients_.back();
		
		sf::Socket::Status AcceptStatus = ListenerSocket_.accept(NewClient->Socket);
		if (AcceptStatus != sf::Socket::Status::Done)
		{
			ConnectedClients_.pop_back();
			NSLOG(NS::ELogLevel::INFO, "[SERVER] Failed to accept connection.");
		}
		else
		{
			NewClient->Socket.setBlocking(false);
			NewClient->ClientId = static_cast<uint16_t>(ConnectedClients_.size() - 1);
			Server_Selector_.add(NewClient->Socket);
			
			--NumClients;
			NSLOG(ELogLevel::INFO, "[SERVER] Accepted connection from {}:{}", 
				NewClient->Socket.getRemoteAddress()->toString(),
				NewClient->Socket.getRemotePort());
			
			// Assign id to new client
			{
				NSLOG(ELogLevel::INFO, "Assigning NetId to Client {}", NewClient->ClientId);
				NetRequest Request;
				Request.Reliability = EReliability::RELIABLE;
				Request.RequestType = ERequestType::ID_ASSIGNMENT;
				Request.InstanceId = static_cast<InstanceIdType>(NewClient->ClientId);
				Request.ActorId = 0;
				Request.ObjectOffset = 0;
				Request.DataSize = 0;
			
				PushRequest(Request);
			}
			
			OnClientConnected(NewClient.get());
		}
	}
}

void NS::Networking::Server_SetMaxConnections(const int NumMaxConnections)
{
	NumMaxConnections_ = NumMaxConnections; 
}

void NS::Networking::Server_AssignOnClientConnected(OnClientConnectedDelegate Callback)
{
	OnClientConnected = Callback;
}

void NS::Networking::Server_SendPackets()
{
	while (!OutgoingPackets_.empty())
	{
		NetRequest Request = OutgoingPackets_.front();
		OutgoingPackets_.pop_front();
		if (Request.Reliability == EReliability::RELIABLE)
		{
			if (Request.InstanceId != -1 && Request.RequestType != ERequestType::ACTOR_CREATION)
			{
				const int ClientIndex = Request.InstanceId;
				sf::TcpSocket& Socket = ConnectedClients_.at(ClientIndex)->Socket;
				sf::Packet Packet;
				Packet << Request;
				if (SendPacketHelper(Packet, Socket) == sf::Socket::Status::Disconnected)
				{
					ConnectedClients_.erase(ConnectedClients_.begin() + ClientIndex);
				}
			}
			else
			{
				ClientVectorType::const_iterator It = ConnectedClients_.begin();
				while (It != ConnectedClients_.end())
				{
					sf::TcpSocket& Socket = (*It)->Socket;
					sf::Packet Packet;
					Packet << Request;
					if (SendPacketHelper(Packet, Socket) == sf::Socket::Status::Disconnected)
					{
						std::lock_guard<std::mutex> QueueLock(QueueMutex_);
						Server_Selector_.remove((*It)->Socket);
						It = ConnectedClients_.erase(It);
					}
					else
					{
						++It;
					}
				}
			}
		}
	}
}

void NS::Networking::Server_ReceivePackets()
{
	if (Server_Selector_.wait(sf::milliseconds(NS::SERVER_SELECTOR_WAIT_TIME_MS)))
	{
		std::lock_guard<std::mutex> QueueLock(QueueMutex_);
		for (auto& SocketPtr : ConnectedClients_)
		{
			if (!Server_Selector_.isReady(SocketPtr->Socket))
			{
				continue;
			}
		
			sf::Packet Packet;
			const auto ReceiveStatus = SocketPtr->Socket.receive(Packet);
			if (ReceiveStatus == sf::Socket::Status::Error)
			{
				NSLOG(LOGERROR, "[CLIENT] Failed to receive packet.");
			}
			else if (ReceiveStatus == sf::Socket::Status::Done)
			{
				NS::NetRequest Request;
				Packet >> Request;
				{
					IncomingPackets_.emplace_back(Request);
				}
			}
		}
	}
}

void NS::Networking::Server_ProcessRequests()
{
	for (ReplicatedProp& Prop : ReplicatedProps_)
	{
		if (!ActorRegistry_.contains(Prop.ActorPtr))
		{
			continue;
		}
		
		void* DataPtr = reinterpret_cast<char*>(Prop.ActorPtr) + Prop.Offset;
		// Property has not changed.
		if (memcmp(Prop.PreviousValue.data(), DataPtr, Prop.Size) == 0)
		{
			continue;
		}
		
		const IdentifierType ActorId = ActorRegistry_.at(Prop.ActorPtr);
		NetRequest ReplicationRequest;
		ReplicationRequest.Reliability = EReliability::RELIABLE;
		ReplicationRequest.RequestType = ERequestType::REPLICATION;
		ReplicationRequest.InstanceId = -1;
		ReplicationRequest.ActorId = ActorId;
		ReplicationRequest.ObjectOffset = Prop.Offset;
		ReplicationRequest.DataSize = Prop.Size;  
		
		memcpy_s(ReplicationRequest.Data, NS::MAX_PACKET_SIZE, DataPtr, Prop.Size);
		memcpy_s(Prop.PreviousValue.data(), 16, DataPtr, Prop.Size);
			
		PushRequest(ReplicationRequest);
	}
	
	std::lock_guard<std::mutex> QueueLock(QueueMutex_);
	while (!IncomingPackets_.empty())
	{
		NetRequest Request = IncomingPackets_.front();
		IncomingPackets_.pop_front();

		switch (Request.RequestType)
		{
			case ERequestType::RPC:
			{
				RPCReceived RpcReceived;
				RpcReceived.ActorId = Request.ActorId;
				memcpy_s(&RpcReceived.FunctionHash, sizeof(size_t), Request.Data, Request.DataSize);
				ProcessRequest_RPCReceived(RpcReceived);
				break;
			}
			default:
				// Server doesn't process any other types of requests.
				break;
		}
	}
}

void NS::Networking::Server_RegisterNewActor(Actor* NewActor, const NS::IdentifierType AuthNetId)
{
	// 1. add unique entry to registry
	IdentifierType NewActorId = LastActorId++;
	ActorRegistry_[NewActor] = NewActorId;
	
	// 2. send packet to Clients
	NetRequest ActorCreationRequest;
	ActorCreationRequest.Reliability = EReliability::RELIABLE;
	ActorCreationRequest.RequestType = ERequestType::ACTOR_CREATION;
	ActorCreationRequest.InstanceId = static_cast<InstanceIdType>(AuthNetId);
	ActorCreationRequest.ActorId = NewActorId;
	ActorCreationRequest.ObjectOffset = 0;
	
	const size_t ActorTypeInfo = NewActor->GetTypeInfo();
	ActorCreationRequest.DataSize = sizeof(ActorTypeInfo);
	memcpy_s(ActorCreationRequest.Data, NS::MAX_PACKET_SIZE, &ActorTypeInfo, ActorCreationRequest.DataSize);
	
	PushRequest(ActorCreationRequest);
}

void NS::Networking::Server_DeRegisterActor(Actor* Actor)
{
	std::vector<ReplicatedProp>::const_iterator It = ReplicatedProps_.begin();
	while (It != ReplicatedProps_.end())
	{
		const ReplicatedProp& Prop = *It;
		if (Prop.ActorPtr == Actor)
		{
			It = ReplicatedProps_.erase(It);
		}
		else
		{
			++It;
		}
	}
	
	NetRequest Request;
	Request.Reliability = EReliability::RELIABLE;
	Request.RequestType = ERequestType::ACTOR_DESTRUCTION;
	Request.InstanceId = -1;
	Request.ActorId = ActorRegistry_.at(Actor);
	Request.ObjectOffset = 0;
	Request.DataSize = 0;
	PushRequest(Request);
	
	ActorRegistry_.erase(Actor);
}

#endif
