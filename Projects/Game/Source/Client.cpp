#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"
#include "Input.h"
#include "World.h"

int main()
{
	NS::Tank* PlayerTank = nullptr;
	NS::Engine* Engine = NS::Engine::Get();
	
	Engine->CreateActor<NS::World>();
	sf::RenderWindow Window(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	sf::View View({NS::SCREEN_WIDTH / 2, NS::SCREEN_HEIGHT / 2}, {NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT});
	
	Engine->StartSubsystems();
	
	while (Window.isOpen() && NS::Networking::Get()->IsConnectedToServer())
	{
		const std::optional<sf::Event> Event = Window.pollEvent();
		if (Event && Window.hasFocus())
		{
			if (Event->is<sf::Event::Closed>())
			{
				Window.close();
			}
			
			NS::Input::Get()->UpdateEvents(Event);
		}
		
		if (Window.hasFocus())
		{
			NS::Input::Get()->UpdateAxes();
		}
		
		if (!PlayerTank)
		{
			PlayerTank = NS::Engine::Get()->GetOwnedActor<NS::Tank>();
		}
		else
		{
			if (!PlayerTank->GetIsPlayerInputIntialized())
			{
				PlayerTank->InitInput();
				PlayerTank->SetWindow(Window);
			}
			
			View.setCenter(PlayerTank->GetPosition());
		}
		
		Engine->Update(0.016f);
		
		// DRAW
		{
			const auto WindowPos = Window.getPosition();
			//NSLOG(NS::ELogLevel::INFO, "Window Position : {},{}", WindowPos.x, WindowPos.y);
			Window.setView(View);
			Window.clear();
			Engine->Draw(Window);
			Window.display();
		}
	}
	
	Engine->StopSubsystems();
	
	if (Window.isOpen())
	{
		Window.close();
	}
	
	std::cin.get(); // prevents the server console from shutting down.
	
	return 0;
}
