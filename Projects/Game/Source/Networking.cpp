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

#ifdef NS_CLIENT
void NS::Networking::Connect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort)
{
	ClientSocket_.disconnect();
	const auto ConnectStatus = ClientSocket_.connect(ServerAddress, ServerPort, sf::seconds(2.0f));
	if (ConnectStatus != sf::Socket::Status::Done)
	{
		NSLOG(NS::ELogLevel::ERROR, "Client - Failed to connect to Server with address {}:{}", ServerAddress.toString(), ServerPort);
	}
	else
	{
		NSLOG(NS::ELogLevel::INFO, "Client - Connected successfully to server at {}:{}", ServerAddress.toString(), ServerPort);
	}
}
#endif

#ifdef NS_SERVER
void NS::Networking::Listen()
{
	ServerSocket_.close();
	NSLOG(NS::ELogLevel::INFO, "Server - Listening for connections on port {}", NS::SERVER_PORT);
	sf::Socket::Status ListenStatus = ServerSocket_.listen(NS::SERVER_PORT);
	if (ListenStatus == sf::Socket::Status::Done)
	{
		PerClientSockets_.emplace_back();
		sf::Socket::Status AcceptStatus = ServerSocket_.accept(PerClientSockets_.back());
	}
}
#endif