#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Input.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"

NS::Engine Engine;
NS::Tank* Tank = nullptr;

void DestroyActor(const sf::Keyboard::Scancode ScanCode)
{
	Engine.DestroyActor(Tank);
}

int main()
{
	Tank = Engine.CreateActor<NS::Tank>();
	NS::Input::Get()->BindOnKeyPressed(sf::Keyboard::Scancode::G, &DestroyActor);
	
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!!NetStrike!!");
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
		NS::Input::Get()->Update(Event);
		Engine.Update(0.016f);
		Engine.Draw(Window);
		Window.display();
	}

	NSLOG(NS::ELogLevel::WARNING, "Application exiting...");
	return 0;
}
