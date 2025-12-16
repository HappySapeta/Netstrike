#include <chrono>
#include <thread>
#include <random>

#include "GameConfiguration.h"
#include "Projectile.h"
#include "Tank.h"
#include "Engine/Engine.h"

typedef std::chrono::high_resolution_clock ChronoClock;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> ChronoTimePoint;
typedef std::chrono::duration<float> ChronoDuration;

NS::Engine* Engine = NS::Engine::Get();
NS::Networking* Networking = NS::Networking::Get();

void PerformCollisions(const std::vector<NS::Actor*>& Actors);
sf::Vector2f GetRandomPosition();
void OnClientConnected(const NS::NetClient* NewClient);
void RubberBand(const std::vector<NS::Actor*>& Actors);

constexpr float COLLISION_RADIUS = 25.0f;
constexpr float DAMAGE = 20.0f;

int main(int argc, char *argv[])
{
	int NumMaxConnections = 1;
	if (argc >= 2)
	{
		NumMaxConnections = atoi(argv[1]);
	}
	Networking->Server_SetMaxConnections(NumMaxConnections);
	Networking->Server_AssignOnClientConnected(&OnClientConnected);
	Engine->StartSubsystems();
	
	float DeltaTime = 0.016f;
	while (Networking->Server_HasConnections())
	{
		const ChronoTimePoint TickStart = ChronoClock::now();
		
		Engine->Update(DeltaTime);
		PerformCollisions(Engine->GetActors());
		RubberBand(Engine->GetActors());

		const ChronoTimePoint TickEnd = ChronoClock::now();
		const ChronoDuration TickDuration = TickEnd - TickStart;
		DeltaTime = TickDuration.count();

		// Limit the tick rate of server
		//{
		//	const auto TickDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(TickDuration);
		//	const auto TimeToWaitMs = NS::SERVER_TICK_DURATION_MS - TickDurationMs.count();
		//	if (TimeToWaitMs > 0)
		//	{
		//		std::this_thread::sleep_for(std::chrono::milliseconds(TimeToWaitMs));
		//	}
		//}
	}
	
	NSLOG(NS::ELogLevel::INFO, "All clients have disconnected. Nothing to serve.");
	
	Engine->StopSubsystems();
	
	std::cin.get(); // prevents the server console from shutting down.
	
	return 0;
}

sf::Vector2f GetRandomPosition()
{
	std::random_device Device;
	std::mt19937 Engine(Device());
	std::uniform_int_distribution<int> DistributionX(0, NS::WORLD_SIZE);
	std::uniform_int_distribution<int> DistributionY(-NS::WORLD_SIZE, 0);
	
	return {static_cast<float>(DistributionX(Engine)), static_cast<float>(DistributionY(Engine))};
}

void OnClientConnected(const NS::NetClient* NewClient)
{
	NSLOG(NS::ELogLevel::INFO, "New client connected : {}", NewClient->ClientId);
	NS::Tank* NewTank = NS::Engine::Get()->CreateActor<NS::Tank>(NewClient->ClientId);
	
	NewTank->SetPosition(GetRandomPosition());
}

void PerformCollisions(const std::vector<NS::Actor*>& Actors)
{
	for (int FirstIndex = 0; FirstIndex < Actors.size(); ++FirstIndex)
	{
		for (int SecondIndex = 0; SecondIndex < Actors.size(); ++SecondIndex)
		{
			if (SecondIndex == FirstIndex)
			{
				continue;
			}
			
			NS::Actor* First = Actors.at(FirstIndex);
			NS::Actor* Second = Actors.at(SecondIndex);
			
			if (First->IsPendingKill() || Second->IsPendingKill())
			{
				continue;
			}
			
			NS::Projectile* Projectile = dynamic_cast<NS::Projectile*>(First);
			if (Projectile == nullptr)
			{
				Projectile = dynamic_cast<NS::Projectile*>(Second);
			}
			
			NS::Tank* Tank = dynamic_cast<NS::Tank*>(First);
			if (Tank == nullptr)
			{
				Tank = dynamic_cast<NS::Tank*>(Second);
			}
			
			if (Tank && Projectile && Projectile->GetParentTank() != Tank)
			{
				const sf::Vector2f TankPos = Tank->GetPosition();
				const sf::Vector2f ProjPos = Projectile->GetPosition();
				
				if ((TankPos - ProjPos).length() < COLLISION_RADIUS)
				{
					Engine->DestroyActor(Projectile);
					Tank->DoDamage(DAMAGE);
				}
			}
		}
	}
}

void RubberBand(const std::vector<NS::Actor*>& Actors)
{
	for (NS::Actor* Actor : Actors)
	{
		sf::Vector2f NewPosition = Actor->GetPosition();
		if (NewPosition.x < 0)
		{
			NewPosition.x = NS::WORLD_SIZE;
		}
		else if (NewPosition.x > NS::WORLD_SIZE)
		{
			NewPosition.x = 0;
		}
		
		if (NewPosition.y > 0)
		{
			NewPosition.y = -NS::WORLD_SIZE;
		}
		else if (NewPosition.y < -NS::WORLD_SIZE)
		{
			NewPosition.y = 0;
		}
		
		Actor->SetPosition(NewPosition);
	}
}