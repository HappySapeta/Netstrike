#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Networking/Networking.h"

int main()
{
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	NSLOG(NS::ELogLevel::INFO, "Created a new window!");

	NS::Networking* Networking = NS::Networking::Get();
	sf::TcpSocket& ClientSocket = Networking->TCPConnect(NS::SERVER_ADDRESS, NS::SERVER_PORT);
	
	while (Window.isOpen())
	{
		const std::optional<sf::Event> Event = Window.pollEvent();
		if (Event)
		{
			if (Event->is<sf::Event::Closed>())
			{
				NSLOG(NS::ELogLevel::INFO, "Shutdown request received. Closing window!");
				Window.close();
			}
		}

		static int value = 0;
		sf::Packet Packet;
		Packet << value++;
		const auto SendStatus = ClientSocket.send(Packet);
		if (SendStatus != sf::Socket::Status::Done)
		{
			NSLOG(NS::ELogLevel::ERROR, "Failed to send packet to server. {}:{}",
				ClientSocket.getRemoteAddress()->toString(), ClientSocket.getRemotePort());
		}
		
		Window.clear();
		Window.display();
	}

	NSLOG(NS::ELogLevel::WARNING, "Application exiting...");
	return 0;
}
