#include <SFML/Graphics.hpp>

#include "Logger.h"
#include "Tank.h"

int main()
{
	NS::Tank Tank;
	
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

		Tank.Update(0.016f);
		Tank.Draw(Window);
		
		Window.display();
	}

	NSLOG(NS::ELogLevel::WARNING, "Application exiting...");
	return 0;
}
