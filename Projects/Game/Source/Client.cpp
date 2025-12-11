#include <random>
#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"

//#define SHOW_ACTIVITY_INDICATOR

sf::Vector2f GetRandomPosition()
{
	std::random_device Device;
	std::mt19937 Engine(Device());
	std::uniform_int_distribution<int> DistributionX(0, NS::SCREEN_WIDTH);
	std::uniform_int_distribution<int> DistributionY(0, NS::SCREEN_HEIGHT);
	
	return {static_cast<float>(DistributionX(Engine)), static_cast<float>(DistributionY(Engine))};
}

int main()
{
	NS::Engine* Engine = NS::Engine::Get();
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	
#ifdef SHOW_ACTIVITY_INDICATOR
	sf::RectangleShape Square({100,100});
	Square.setPosition({NS::SCREEN_WIDTH / 2.0f, NS::SCREEN_HEIGHT / 2.0f});
	Square.setFillColor(sf::Color::White);
#endif
	
	Engine->StartSubsystems();
	NS::Tank* PlayerTank = nullptr;
	
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
		
		if (PlayerTank)
		{
			PlayerTank->RPC_MoveRandom();
		}
		else
		{
			const std::vector<NS::Actor*>& Actors = Engine->GetActors();
			for (NS::Actor* Actor : Actors)
			{
				if (Actor->GetNetId() == NS::Networking::Get()->Client_GetNetId())
				{
					if (NS::Tank* Ptr = dynamic_cast<NS::Tank*>(Actor))
					{
						PlayerTank = Ptr;
					}
				}
			}
		}
		
		Engine->Update(0.016f);
		
		// DRAW
		{
			Window.clear();
			Engine->Draw(Window);
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
	
	Engine->StopSubsystems();
	
	std::cin.get();
	return 0;
}
