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
static bool IsBot = false;
std::string WindowTitle = "!! N E T S T R I K E !!";
int PortNumber = NS::SERVER_PORT;

void Initialize();
void ParseCommandArgs(int argc, char** argv);
void UpdatePlayerTank();
void UpdateInput();

int main(int argc, char* argv[])
{
	ParseCommandArgs(argc, argv);
	Initialize();
	NS::Networking::Get()->Client_SetPortNumber(PortNumber);
	Engine->StartSubsystems();
	
	float DeltaTime = 0.016f;
	while (Window->isOpen() && NS::Networking::Get()->IsConnectedToServer())
	{
		const ChronoTimePoint TickStart = ChronoClock::now();
		UpdateInput();
		UpdatePlayerTank();
		
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
	
	Window = std::make_unique<sf::RenderWindow>(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), WindowTitle);
	Window->setVerticalSyncEnabled(false);
	
	View = std::make_unique<sf::View>
	(
		sf::Vector2f{NS::SCREEN_WIDTH / 2, NS::SCREEN_HEIGHT / 2}, 
		sf::Vector2f{NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}
	);
	
}

void ParseCommandArgs(int argc, char* argv[])
{
	if (argc >= 2)
	{
		const std::string Mode = argv[1];
		if (Mode == "bot")
		{
			WindowTitle += " BOT";
			NSLOG(NS::ELogLevel::INFO, "Launching game as bot.");
			IsBot = true;
		}
		else if (Mode == "player")
		{
			WindowTitle += " PLAYER";
			NSLOG(NS::ELogLevel::INFO, "Launching game as player.");
			IsBot = false;
		}
	}
	else
	{
		WindowTitle += " PLAYER";
		NSLOG(NS::ELogLevel::INFO, "Launching game as player.");
		IsBot = false;
	}
	
	if (argc >= 3)
	{
		PortNumber = atoi(argv[2]);
	}
}

void UpdatePlayerTank()
{
	if (!PlayerTank)
	{
		PlayerTank = NS::Engine::Get()->GetOwnedActor<NS::Tank>();
		if (PlayerTank)
		{
			PlayerTank->InitInput(IsBot);
		}
	}
	else
	{
		View->setCenter(PlayerTank->GetPosition());
	}
}

void UpdateInput()
{
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
}
