#ifdef NS_SERVER

#include "Logger.h"
#include "Actor/Actor.h"
#include "Networking/Networking.h"

// TODO: Use Non-blocking sockets if possible.
void NS::Networking::Server_Listen()
{
	ListenerSocket_.close();
	NSLOG(NS::ELogLevel::INFO, "Listening for connections on port {}", NS::SERVER_PORT);
	const sf::Socket::Status ListenStatus = ListenerSocket_.listen(NS::SERVER_PORT);
	
	if (ListenStatus != sf::Socket::Status::Done)
	{
		NSLOG(ELogLevel::ERROR, "[SERVER] Failed to listen on port {}.", NS::SERVER_PORT);
		return;
	}

	int NumClients = NS::DEBUG_SERVER_MAX_CONNECTIONS;
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
		}
	}
}

void NS::Networking::Server_SendPackets()
{
	while (!OutgoingPackets_.empty())
	{
		NetRequest Request = OutgoingPackets_.front();
		OutgoingPackets_.pop_front();
		{
			if (Request.Reliability == EReliability::RELIABLE)
			{
				sf::TcpSocket& Socket = ConnectedClients_.at(Request.InstanceId)->Socket; // TODO : Major bug here. InstanceId can be -1.
				sf::Packet Packet;
				Packet << Request;
				SendPacketHelper(Packet, Socket);
			}
		}
	}
}

void NS::Networking::Server_ReceivePackets()
{
	if (Server_Selector_.wait(sf::milliseconds(NS::SERVER_SELECTOR_WAIT_TIME_MS)))
	{
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
				IncomingPackets_.emplace_back(Request);
			}
		}
	}
}

void NS::Networking::Server_ProcessRequests()
{
	for (const ReplicatedProp& Prop : ReplicatedProps_)
	{
		if (!ActorRegistry_.contains(Prop.ActorPtr))
		{
			continue;
		}
		
		const IdentifierType ActorId = ActorRegistry_.at(Prop.ActorPtr);
		NetRequest ReplicationRequest;
		ReplicationRequest.Reliability = EReliability::RELIABLE;
		ReplicationRequest.RequestType = ERequestType::REPLICATION;
		ReplicationRequest.InstanceId = 0; // TODO : Handle multiple clients
		ReplicationRequest.ActorId = ActorId;
		ReplicationRequest.ObjectOffset = Prop.Offset;
		ReplicationRequest.DataSize = Prop.Size;  
		
		void* DataPtr = reinterpret_cast<char*>(Prop.ActorPtr) + Prop.Offset;
		memcpy_s(ReplicationRequest.Data, NS::MAX_PACKET_SIZE, DataPtr, Prop.Size);
			
		PushRequest(ReplicationRequest);
	}
	
	while (!IncomingPackets_.empty())
	{
		NetRequest Packet = IncomingPackets_.front();
		IncomingPackets_.pop_front();

		switch (Packet.RequestType)
		{
			case ERequestType::RPC:
			{
				RPCRequest RpcRequest;
				ProcessRequest_RPC(RpcRequest);
				break;
			}
			default:
				// Server doesn't process any other types of requests.
				break;
		}
	}
}

void NS::Networking::Server_RegisterNewActor(Actor* NewActor)
{
	// 1. add unique entry to registry
	IdentifierType NewActorId = LastActorId++;
	ActorRegistry_[NewActor] = NewActorId;
	
	// 2. send packet to Clients
	NetRequest ActorCreationRequest;
	ActorCreationRequest.Reliability = EReliability::RELIABLE;
	ActorCreationRequest.RequestType = ERequestType::ACTOR_CREATION;
	ActorCreationRequest.InstanceId = 0;
	ActorCreationRequest.ActorId = NewActorId;
	ActorCreationRequest.ObjectOffset = 0;
	
	const size_t ActorTypeInfo = NewActor->GetTypeInfo();
	ActorCreationRequest.DataSize = sizeof(ActorTypeInfo);
	memcpy_s(ActorCreationRequest.Data, NS::MAX_PACKET_SIZE, &ActorTypeInfo, ActorCreationRequest.DataSize);
	
	PushRequest(ActorCreationRequest);
}

#endif
