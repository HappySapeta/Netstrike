#ifdef NS_SERVER

#include "Logger.h"
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

void NS::Networking::Server_ReplicateToClient(const ReplicatedProp& Property, const IdentifierType InstanceId)
{
	NS::NetPacket RepRequest;
	RepRequest.Reliability = EReliability::RELIABLE;
	RepRequest.InstanceId = InstanceId;
	RepRequest.Property = Property;
	memcpy_s(RepRequest.Data, NS::MAX_PACKET_SIZE, Property.DataPtr, Property.Size);
	
	PushRequest(RepRequest);
}
#endif