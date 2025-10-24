#include <SFML/Graphics.hpp>

#include "Logger.h"

int main()
{
	sf::RenderWindow Window(sf::VideoMode({1280, 720}), "!!NetStrike!!");
	NSLOG(NS::ELogLevel::INFO, "Created a new window!");
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
		Window.display();
	}

	NSLOG(NS::ELogLevel::WARNING, "Application exiting...");
	return 0;
}
