#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Networking/Networking.h"
#include "Test/ReplicationTest.h"


int main()
{
	NS::ReplicationTest Test;
	
	
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	NSLOG(NS::ELogLevel::INFO, "Created a new window!");

	NS::Networking* Networking = NS::Networking::Get();
	Networking->TCPConnect(NS::SERVER_ADDRESS, NS::SERVER_PORT);
	
	
	Networking->Client_ReplicateFromServer(&Test.Variable, sizeof(Test.Variable), 0);
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
		
		NSLOG(NS::ELogLevel::INFO, "[CLIENT] Test.Variable = {}", Test.Variable);
		Window.display();
	}

	Networking->Stop();
	NSLOG(NS::ELogLevel::WARNING, "Application exiting...");
	
	std::cin.get();
	return 0;
}
