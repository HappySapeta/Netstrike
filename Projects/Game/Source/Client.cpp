#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"
#include "Input.h"


int main()
{
	NS::Tank* PlayerTank = nullptr;
	NS::Engine* Engine = NS::Engine::Get();
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	
	Engine->StartSubsystems();
	
	while (Window.isOpen())
	{
		const std::optional<sf::Event> Event = Window.pollEvent();
		if (Event && Window.hasFocus())
		{
			if (Event->is<sf::Event::Closed>())
			{
				Window.close();
			}
			
			NS::Input::Get()->Update(Event);
		}
		
		if (!PlayerTank)
		{
			PlayerTank = NS::Engine::Get()->GetOwnedActor<NS::Tank>();
		}
		else
		{
			PlayerTank->InitInput();
		}
		
		Engine->Update(0.016f);
		
		// DRAW
		{
			Window.clear();
			Engine->Draw(Window);
			Window.display();
		}
	}
	
	Engine->StopSubsystems();
	
	std::cin.get();
	return 0;
}
