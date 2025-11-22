#include "Networking/Networking.h"

#include <functional>

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

void NS::Networking::Client_ProcessRequest(NS::NetPacket Request)
{
	const ReplicationObject& ReplObject = Unmap(Request.ObjectId);
	if (ReplObject.DataPtr)
	{
		memcpy(ReplObject.DataPtr, Request.Data, ReplObject.Size);
	}
	else
	{
		NSLOG(ELogLevel::ERROR, "[CLIENT] Failed to unmap ObjectId {}.", Request.ObjectId);
	}
}

void NS::Networking::Client_SendPackets()
{
	while (!OutgoingRequests_.empty())
	{
		NS::NetPacket Request = OutgoingRequests_.front();
		OutgoingRequests_.pop_front();
		
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
				NSLOG(LOGINFO, "[CLIENT] Received packet from server.");
				NS::NetPacket Request;
				Packet >> Request;
				IncomingRequests_.emplace_back(Request);
			}
		}
	}
		
	while (!IncomingRequests_.empty())
	{
		NetPacket Request = IncomingRequests_.front();
		IncomingRequests_.pop_front();
		Client_ProcessRequest(Request);
	}
}

void NS::Networking::Client_ReplicateFromServer(void* Data, uint16_t Size, const uint32_t ObjectId)
{
	ReplObjectMap_[ObjectId] = {Data, Size};
}

#endif

#ifdef NS_SERVER
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
	while (!OutgoingRequests_.empty())
	{
		NetPacket Request = OutgoingRequests_.front();
		OutgoingRequests_.pop_front();
		{
			if (Request.Reliability == EReliability::RELIABLE)
			{
				sf::TcpSocket& Socket = ConnectedClients_.at(Request.InstanceId)->Socket;
				sf::Packet Packet;
				Packet << Request;
				const auto SendStatus = Socket.send(Packet);
				if (SendStatus == sf::Socket::Status::Error)
				{
					NSLOG(ELogLevel::ERROR, "[SERVER] Failed to send packet. {}:{}", Socket.getRemoteAddress()->toString(), Socket.getRemotePort());
				}
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
				NS::NetPacket Request;
				Packet >> Request;
				IncomingRequests_.emplace_back(Request);
			}
		}
	}
		
	while (!IncomingRequests_.empty())
	{
		NetPacket Request = IncomingRequests_.front();
		IncomingRequests_.pop_front();
		Server_ProcessRequest(Request);
	}
}

// TODO: Server_ProcessRequest implementation.
void NS::Networking::Server_ProcessRequest(const NetPacket& Request)
{}

void NS::Networking::Server_ReplicateToClient(const void* Data, const uint16_t Size, const uint32_t ObjectId, uint8_t ClientId)
{
	NS::NetPacket RepRequest;
	RepRequest.Reliability = EReliability::RELIABLE;
	RepRequest.ObjectId = ObjectId;
	RepRequest.Size = Size;
	RepRequest.InstanceId = ClientId;
	memcpy(RepRequest.Data, Data, NS::PACKET_SIZE);
	
	PushRequest(RepRequest);
}
#endif

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

NS::ReplicationObject NS::Networking::Unmap(uint32_t ObjectId)
{
	if (ReplObjectMap_.contains(ObjectId))
	{
		return ReplObjectMap_[ObjectId];
	}

	return {nullptr, 0};
}
