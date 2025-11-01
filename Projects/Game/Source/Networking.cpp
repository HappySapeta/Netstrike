#include "Networking/Networking.h"

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
		NSLOG(NS::ELogLevel::ERROR, "Failed to connect to Server with address {}:{}", ServerAddress.toString(), ServerPort);
	}
}
