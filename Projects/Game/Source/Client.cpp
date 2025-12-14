#include <SFML/Graphics.hpp>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"
#include "Input.h"
#include "World.h"

typedef std::chrono::high_resolution_clock ChronoClock;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> ChronoTimePoint;
typedef std::chrono::duration<float> ChronoDuration;

NS::Tank* PlayerTank = nullptr;
NS::Engine* Engine = nullptr;
std::unique_ptr<sf::RenderWindow> Window;
std::unique_ptr<sf::View> View;

void Initialize();

int main()
{
	Initialize();
	
	Engine->StartSubsystems();
	
	float DeltaTime = 0.016f;
	while (Window->isOpen() && NS::Networking::Get()->IsConnectedToServer())
	{
		const ChronoTimePoint TickStart = ChronoClock::now();
		const std::optional<sf::Event> Event = Window->pollEvent();
		if (Event && Window->hasFocus())
		{
			if (Event->is<sf::Event::Closed>())
			{
				Window->close();
			}
			
			NS::Input::Get()->UpdateEvents(Event);
		}
		
		if (Window->hasFocus())
		{
			NS::Input::Get()->UpdateAxes();
		}
		
		if (!PlayerTank)
		{
			PlayerTank = NS::Engine::Get()->GetOwnedActor<NS::Tank>();
		}
		else
		{
			if (!PlayerTank->GetIsInputInitalized())
			{
				PlayerTank->InitInput();
			}
			
			View->setCenter(PlayerTank->GetPosition());
		}
		
		Engine->Update(DeltaTime);
		
		// DRAW
		{
			Window->setView(*View.get());
			Window->clear();
			Engine->Draw(*Window.get());
			Window->display();
		}
		
		const ChronoTimePoint TickEnd = ChronoClock::now();
		const ChronoDuration TickDuration = (TickEnd - TickStart);
		DeltaTime = TickDuration.count();
	}
	
	Engine->StopSubsystems();
	
	if (Window->isOpen())
	{
		Window->close();
	}
	
	std::cin.get(); // prevents the server console from shutting down.
	
	return 0;
}

void Initialize()
{
	Engine = NS::Engine::Get();
	Engine->CreateActor<NS::World>();
	
	Window = std::make_unique<sf::RenderWindow>(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), "!! N E T S T R I K E !!");
	Window->setVerticalSyncEnabled(false);
	
	View = std::make_unique<sf::View>
	(
		sf::Vector2f{NS::SCREEN_WIDTH / 2, NS::SCREEN_HEIGHT / 2}, 
		sf::Vector2f{NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}
	);
	
}
