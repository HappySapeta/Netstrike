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
sf::TcpSocket& NS::Networking::TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort)
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
	
	return TCPSocket_;
}

void NS::Networking::Client_ProcessRequest(NS::NetRequest Request)
{
	if (Request.InstructionType == EInstructionType::REPLICATION)
	{
		const ReplicationObject& ReplObject = Unmap(Request.ObjectId);
		if (ReplObject.DataPtr)
		{
			memcpy(ReplObject.DataPtr, Request.Data, ReplObject.Size);
		}
	}
}

void NS::Networking::Client_SendPackets()
{
	while (!OutgoingRequests_.empty())
	{
		NS::NetRequest Request = OutgoingRequests_.front();
		OutgoingRequests_.pop_front();
		
		sf::Packet Packet;
		Packet << Request;
		
		if (Request.Reliability == EReliability::RELIABLE)
		{
			const auto SendStatus = TCPSocket_.send(Packet);
			if (SendStatus != sf::Socket::Status::Done)
			{
				NSLOG(ELogLevel::ERROR, "Failed to send packet to server!");
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
				NS::NetRequest Request;
				Packet >> Request;
				IncomingRequests_.emplace_back(Request);
			}
		}
	}
		
	while (!IncomingRequests_.empty())
	{
		NetRequest Request = IncomingRequests_.front();
		IncomingRequests_.pop_front();
		Client_ProcessRequest(Request);
	}
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
		ConnectedClientSockets_.emplace_back(std::make_unique<sf::TcpSocket>());
		sf::TcpSocket& NewSocket = *ConnectedClientSockets_.back();
		
		sf::Socket::Status AcceptStatus = ListenerSocket_.accept(NewSocket);
		if (AcceptStatus != sf::Socket::Status::Done)
		{
			ConnectedClientSockets_.pop_back();
			NSLOG(NS::ELogLevel::INFO, "Failed to accept connection.");
		}
		else
		{
			NewSocket.setBlocking(false);
			Server_Selector_.add(NewSocket);
			
			--NumClients;
			NSLOG(ELogLevel::INFO, "Accepted connection from {}:{}", NewSocket.getRemoteAddress()->toString(), NewSocket.getRemotePort());
		}
	}
}

void NS::Networking::Server_SendPackets()
{
	while (!OutgoingRequests_.empty())
	{
		NetRequest Request = OutgoingRequests_.front();
		OutgoingRequests_.pop_front();
		NSLOG(LOGINFO, "[SERVER] Sending packet");
		{
			if (Request.Reliability == EReliability::RELIABLE)
			{
				sf::TcpSocket& Socket = *ConnectedClientSockets_.at(Request.InstanceId);
				sf::Packet Packet;
				Packet << Request;
				const auto SendStatus = Socket.send(Packet);
				if (SendStatus == sf::Socket::Status::Error)
				{
					NSLOG(ELogLevel::ERROR, "Failed to send packet. {}:{}", Socket.getRemoteAddress()->toString(), Socket.getRemotePort());
				}
			}
		}
	}
}

void NS::Networking::Server_ReceivePackets()
{
	if (Server_Selector_.wait(sf::milliseconds(NS::SERVER_SELECTOR_WAIT_TIME_MS)))
	{
		for (auto& SocketPtr : ConnectedClientSockets_)
		{
			if (!Server_Selector_.isReady(*SocketPtr))
			{
				continue;
			}
		
			sf::Packet Packet;
			const auto ReceiveStatus = SocketPtr->receive(Packet);
			if (ReceiveStatus == sf::Socket::Status::Error)
			{
				NSLOG(LOGERROR, "[CLIENT] Failed to receive packet.");
			}
			else if (ReceiveStatus == sf::Socket::Status::Done)
			{
				NS::NetRequest Request;
				Packet >> Request;
				IncomingRequests_.emplace_back(Request);
			}
		}
	}
		
	while (!IncomingRequests_.empty())
	{
		NetRequest Request = IncomingRequests_.front();
		IncomingRequests_.pop_front();
		Server_ProcessRequest(Request);
	}
}

void NS::Networking::Server_ProcessRequest(const NetRequest& Request)
{
	if (Request.InstructionType == EInstructionType::REPLICATION)
	{
		return;
	}
	
	if (Request.InstructionType == EInstructionType::RPC)
	{
		std::string Message(Request.Data);
		NSLOG(ELogLevel::INFO, "[SERVER] Received RPC request from client. {}", Message);
	}
	
	// perform RPC call.
}
#endif

void NS::Networking::PushRequest(const NetRequest& NewRequest)
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

void NS::Networking::ProcessRequests()
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

NS::ReplicationObject NS::Networking::Unmap(uint32_t ObjectId)
{
	if (ReplObjectMap_.contains(ObjectId))
	{
		return ReplObjectMap_[ObjectId];
	}

	return {nullptr, 0};
}
