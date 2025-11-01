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

void NS::Networking::Connect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort)
{
	MainSocket_.disconnect();
	const auto ConnectStatus = MainSocket_.connect(ServerAddress, ServerPort, sf::seconds(2.0f));
	if (ConnectStatus != sf::Socket::Status::Done)
	{
		NSLOG(NS::ELogLevel::ERROR, "Client - Failed to connect to Server with address {}:{}", ServerAddress.toString(), ServerPort);
	}
	else
	{
		NSLOG(NS::ELogLevel::INFO, "Client - Connected successfully to server at {}:{}", ServerAddress.toString(), ServerPort);
	}
}

void NS::Networking::Listen()
{
	MainListener_.close();
	NSLOG(NS::ELogLevel::INFO, "Server - Listening for connections on port {}", NS::SERVER_PORT);
	sf::Socket::Status ListenStatus = MainListener_.listen(NS::SERVER_PORT);
	if (ListenStatus == sf::Socket::Status::Done)
	{
		ClientSockets_.emplace_back();
		sf::Socket::Status AcceptStatus = MainListener_.accept(ClientSockets_.back());
	}
}
