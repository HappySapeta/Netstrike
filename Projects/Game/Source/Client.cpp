#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Engine/Engine.h"

static NS::Engine Engine;

#define SHOW_ACTIVITY_INDICATOR

int main()
{
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	
#ifdef SHOW_ACTIVITY_INDICATOR
	sf::RectangleShape Square({100,100});
	Square.setPosition({NS::SCREEN_WIDTH / 2.0f, NS::SCREEN_HEIGHT / 2.0f});
	Square.setFillColor(sf::Color::White);
#endif
	
	Engine.StartSubsystems();
	
	while (Window.isOpen())
	{
		const std::optional<sf::Event> Event = Window.pollEvent();
		if (Event)
		{
			if (Event->is<sf::Event::Closed>())
			{
				Window.close();
			}
		}
		
		Engine.Update(0.016f);
		
		// DRAW
		{
			Window.clear();
			Engine.Draw(Window);
#ifdef SHOW_ACTIVITY_INDICATOR
			// Activity indicator
			{
				Square.rotate(sf::degrees(1.0f));
				Window.draw(Square);
			}
#endif
			Window.display();
		}
	}
	
	Engine.StopSubsystems();
	
	std::cin.get();
	return 0;
}
