#include <SFML/Graphics.hpp>

int main()
{
	sf::RenderWindow Window(sf::VideoMode({1280, 720}), "!!NetStrike!!");

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
		
		Window.clear();
		Window.display();
	}

	
	return 0;
}