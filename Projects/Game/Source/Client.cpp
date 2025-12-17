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

constexpr float MAX_FRAME_RATE = 120;
NS::Tank* PlayerTank = nullptr;
NS::Engine* Engine = nullptr;
std::unique_ptr<sf::RenderWindow> Window;
std::unique_ptr<sf::View> View;
static bool IsBot = false;
std::string WindowTitle = "!! N E T S T R I K E !!";
int PortNumber = NS::SERVER_PORT;
bool bShouldPerformInterpolation = true;

void Initialize();
void ParseCommandArgs(int argc, char** argv);
void UpdatePlayerTank();
void UpdateInput();
void OnActorCreated(NS::Actor*);

int main(int argc, char* argv[])
{
	ParseCommandArgs(argc, argv);
	Initialize();
	NS::Networking::Get()->Client_SetPortNumber(PortNumber);
	Engine->StartSubsystems();
	
	float DeltaTime = 0.016f;
	bool bShouldUpdate;
	do
	{
		bShouldUpdate = (IsBot || Window->isOpen()) && NS::Networking::Get()->IsConnectedToServer();
		const ChronoTimePoint TickStart = ChronoClock::now();
		UpdateInput();
		UpdatePlayerTank();
		
		Engine->Update(DeltaTime);
		
		// DRAW
		if (Window)
		{
			Window->setView(*View.get());
			Window->clear();
			Engine->Draw(*Window.get());
			Window->display();
		}
		
		const ChronoTimePoint TickEnd = ChronoClock::now();
		const ChronoDuration TickDuration = (TickEnd - TickStart);
		DeltaTime = TickDuration.count();
		
		// Limit the tick rate of the client
		if (IsBot)
		{
			const auto TickDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(TickDuration);
			const auto TimeToWaitMs = 8 - TickDurationMs.count();
			if (TimeToWaitMs > 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(TimeToWaitMs));
			}
		}
	}while (bShouldUpdate);
	
	Engine->StopSubsystems();
	
	if (Window && Window->isOpen())
	{
		Window->close();
	}
	
	std::cin.get(); // prevents the server console from shutting down.
	
	return 0;
}

void OnActorCreated(NS::Actor* Actor)
{
	if (NS::Tank* Tank = dynamic_cast<NS::Tank*>(Actor))
	{
		Tank->SetShouldPerformInterpolation(bShouldPerformInterpolation);
	}
}

void Initialize()
{
	Engine = NS::Engine::Get();
	Engine->AssignOnActorCreated(OnActorCreated);
	Engine->CreateActor<NS::World>();
	
	if (IsBot)
	{
		return;
	}
	
	Window = std::make_unique<sf::RenderWindow>(sf::VideoMode({NS::SCREEN_WIDTH, NS::SCREEN_HEIGHT}), WindowTitle);
	Window->setVerticalSyncEnabled(false);
	Window->setFramerateLimit(static_cast<unsigned int>(MAX_FRAME_RATE));
	
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
	
	if (argc >= 4)
	{
		int Value = atoi(argv[3]);
		bShouldPerformInterpolation = Value == 0 ? false : true;
		if (bShouldPerformInterpolation)
		{
			NSLOG(NS::ELogLevel::INFO, "Interpolation is ON.")
		}
		else
		{
			NSLOG(NS::ELogLevel::INFO, "Interpolation is OFF.")
		}
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
		if (View)
		{
			View->setCenter(PlayerTank->GetInterpolatedPosition());
		}
	}
}

void UpdateInput()
{
	if (!Window)
	{
		return;
	}
	
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
