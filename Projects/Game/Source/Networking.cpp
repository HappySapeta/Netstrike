#include "Networking/Networking.h"

#include "GameConfiguration.h"
#include "Logger.h"

std::unique_ptr<NS::Networking> NS::Networking::Instance_(nullptr);

NS::Networking::Networking()
	:
#ifdef NS_SERVER
	NetIdentity_(ENetAuthority::SERVER)
#endif
#ifdef NS_CLIENT
	NetIdentity_(ENetAuthority::CLIENT)
#endif
{}

#ifdef NS_CLIENT
// TODO: Use Non-blocking sockets if possible.
sf::TcpSocket& NS::Networking::TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort)
{
	ServerSocket_.disconnect();
	const auto ConnectStatus = ServerSocket_.connect(ServerAddress, ServerPort, sf::seconds(NS::DEFAULT_CONNECTION_TIMEOUT));
	if (ConnectStatus != sf::Socket::Status::Done)
	{
		NSLOG(NS::ELogLevel::ERROR, "Failed to connect to Server with address {}:{}", ServerAddress.toString(), ServerPort);
	}
	else
	{
		NSLOG(NS::ELogLevel::INFO, "Connected successfully to server at {}:{}", ServerAddress.toString(), ServerPort);
	}

	return ServerSocket_;
}
#endif

#ifdef NS_SERVER
// TODO: Use Non-blocking sockets if possible.
sf::TcpListener& NS::Networking::TCPListen()
{
	ListenerSocket_.close();
	NSLOG(NS::ELogLevel::INFO, "Listening for connections on port {}", NS::SERVER_PORT);
	sf::Socket::Status ListenStatus = ListenerSocket_.listen(NS::SERVER_PORT);
	if (ListenStatus == sf::Socket::Status::Done)
	{
		ConnectedClientSockets_.emplace_back();
		sf::Socket::Status AcceptStatus = ListenerSocket_.accept(ConnectedClientSockets_.back());
		if (AcceptStatus != sf::Socket::Status::Done)
		{
			NSLOG(NS::ELogLevel::INFO, "Failed to accept connection.");
		}
	}

	return ListenerSocket_;
}
#endif

void NS::Networking::PushRequest(const NetRequest& NewRequest)
{
	OutgoingRequests_.push_back(NewRequest);
}

void NS::Networking::Update()
{
	ProcessRequests();
}

void NS::Networking::ProcessRequests()
{
	// Populate the queue with incoming requests
#ifdef NS_CLIENT
	{
		sf::Packet Packet;
		const auto ReceiveStatus = ServerSocket_.receive(Packet);
		if (ReceiveStatus != sf::Socket::Status::Done)
		{
			NSLOG(LOGERROR, "[CLIENT] Failed to receive packet.");
		}
		else 
		{
			NS::NetRequest Request;
			Packet >> Request;
			IncomingRequests_.emplace_back(Request);
		}
	}
#endif NS_CLIENT

	while (!IncomingRequests_.empty())
	{
		NetRequest Request = IncomingRequests_.front();
		NSLOG(LOGINFO, "[CLIENT] Processing packet");
		IncomingRequests_.pop_front();

#ifdef NS_CLIENT
		{
			if (Request.InstructionType == EInstructionType::REPLICATION && Request.NetSource == ENetAuthority::SERVER)
			{
				const ReplicationObject& ReplObject = Unmap(Request.ObjectId);
				if (ReplObject.DataPtr)
				{
					memcpy(ReplObject.DataPtr, Request.Data, ReplObject.Size);
				}
			}
		}
#endif
	}

	while (!OutgoingRequests_.empty())
	{
		NetRequest Request = OutgoingRequests_.front();
		OutgoingRequests_.pop_front();
		NSLOG(LOGINFO, "[SERVER] Sending packet");
#ifdef NS_SERVER
		{
			if (Request.Reliability == EReliability::RELIABLE)
			{
				sf::TcpSocket& Socket = ConnectedClientSockets_.at(Request.InstanceId);

				sf::Packet Packet;
				Packet << Request;
				const auto SendStatus = Socket.send(Packet);
				if (SendStatus != sf::Socket::Status::Done)
				{
					NSLOG(ELogLevel::ERROR, "Failed to send packet. {}:{}",
						Socket.getRemoteAddress()->toString(), Socket.getRemotePort());
				}
			}
		}
#endif
	}
}

NS::ReplicationObject NS::Networking::Unmap(uint32_t ObjectId)
{
	if (ReplObjectMap_.contains(ObjectId))
	{
		return ReplObjectMap_[ObjectId];
	}

	return {nullptr, 0};
}
