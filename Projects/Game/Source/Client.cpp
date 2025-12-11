#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"
#include "Input.h"

NS::Tank* PlayerTank = nullptr;

void MoveTankVertical(const float Input)
{
	if (PlayerTank)
	{
		if (Input > 0)
		{
			PlayerTank->MoveForward();
		}
		else if (Input < 0)
		{
			PlayerTank->MoveBackward();
		}
	}
}

void TurnTank(const float Input)
{
	if (PlayerTank)
	{
		if (Input > 0)
		{
			PlayerTank->TurnRight();
		}
		else if (Input < 0)
		{
			PlayerTank->TurnLeft();
		}
	}
}

int main()
{
	NS::Engine* Engine = NS::Engine::Get();
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	
	Engine->StartSubsystems();
	
	NS::Input* Input = NS::Input::Get();
	Input->BindAxisVertical(&MoveTankVertical);
	Input->BindAxisHorizontal(&TurnTank);
	
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
