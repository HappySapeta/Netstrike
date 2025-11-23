#ifdef NS_CLIENT

#include "Logger.h"
#include "Networking/Networking.h"

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
	const auto& [SourcePtr, TargetPtr, SizeInBytes] = Unmap(Request.Property);
	if (SourcePtr && TargetPtr)
	{
		memcpy(TargetPtr, SourcePtr, SizeInBytes);
	}
	else
	{
		NSLOG(ELogLevel::ERROR, "[CLIENT] Failed to unmap ObjectId.");
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
	ReplicationMap_[ObjectId] = {Data, Size};
}

#endif