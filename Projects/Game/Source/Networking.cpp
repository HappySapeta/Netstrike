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

// TODO: Use Non-blocking sockets if possible.
sf::TcpSocket& NS::Networking::TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort)
{
	ClientSocket_.disconnect();
	const auto ConnectStatus = ClientSocket_.connect(ServerAddress, ServerPort, sf::seconds(NS::DEFAULT_CONNECTION_TIMEOUT));
	if (ConnectStatus != sf::Socket::Status::Done)
	{
		NSLOG(NS::ELogLevel::ERROR, "Failed to connect to Server with address {}:{}", ServerAddress.toString(), ServerPort);
	}
	else
	{
		NSLOG(NS::ELogLevel::INFO, "Connected successfully to server at {}:{}", ServerAddress.toString(), ServerPort);
	}

	return ClientSocket_;
}

// TODO: Use Non-blocking sockets if possible.
sf::TcpListener& NS::Networking::TCPListen()
{
	ServerSocket_.close();
	NSLOG(NS::ELogLevel::INFO, "Listening for connections on port {}", NS::SERVER_PORT);
	sf::Socket::Status ListenStatus = ServerSocket_.listen(NS::SERVER_PORT);
	if (ListenStatus == sf::Socket::Status::Done)
	{
		PerClientSockets_.emplace_back();
		sf::Socket::Status AcceptStatus = ServerSocket_.accept(PerClientSockets_.back());
		if (AcceptStatus != sf::Socket::Status::Done)
		{
			NSLOG(NS::ELogLevel::INFO, "Failed to accept connection.");
		}
	}

	return ServerSocket_;
}

std::vector<sf::TcpSocket>& NS::Networking::GetClientSockets()
{
	return PerClientSockets_;
}
