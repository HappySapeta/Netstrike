#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Networking/Networking.h"

int main()
{
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	NSLOG(NS::ELogLevel::INFO, "Created a new window!");

	NS::Networking* Networking = NS::Networking::Get();
	const auto& Socket = Networking->TCPConnect(NS::SERVER_ADDRESS, NS::SERVER_PORT);
	Networking->Start();
	
	// Activity indiciator square -------
	sf::RectangleShape Square({100,100});
	Square.setPosition({NS::SCREEN_WIDTH / 2.0f, NS::SCREEN_HEIGHT / 2.0f});
	Square.setFillColor(sf::Color::White);
	// -----------------------------------
	
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
		
		Window.clear();
		// Activity indicator
		{
			Square.rotate(sf::degrees(1.0f));
			Window.draw(Square);
		}
		Window.display();
	}

	Networking->Stop();
	NSLOG(NS::ELogLevel::WARNING, "Application exiting...");
	return 0;
}
