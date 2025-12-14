#include <chrono>
#include <thread>
#include <random>

#include "GameConfiguration.h"
#include "Projectile.h"
#include "Tank.h"
#include "Engine/Engine.h"

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;
constexpr float COLLISION_RADIUS = 100.0f;
constexpr float DAMAGE = 20.0f;

NS::Engine* Engine = NS::Engine::Get();
NS::Networking* Networking = NS::Networking::Get();

sf::Vector2f GetRandomPosition()
{
	std::random_device Device;
	std::mt19937 Engine(Device());
	std::uniform_int_distribution<int> DistributionX(0, 200);
	std::uniform_int_distribution<int> DistributionY(-200, 0);
	
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
	
	while (Networking->Server_HasConnections())
	{
		static TimePoint TickStart;
		TickStart = HRClock::now();
		
		Engine->Update(0.016f);
		PerformCollisions(Engine->GetActors());

		Duration TickDuration = HRClock::now() - TickStart;
		DeltaTimeSecs = TickDuration.count();

		const auto TickDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(TickDuration);
		const auto TimeToWaitMs = NS::SERVER_TICK_DURATION_MS - TickDurationMs.count();
		if (TimeToWaitMs > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(TimeToWaitMs));
		}
	}
	
	NSLOG(NS::ELogLevel::INFO, "All clients have disconnected. Nothing to serve.");
	
	Engine->StopSubsystems();
	
	std::cin.get(); // prevents the server console from shutting down.
	
	return 0;
}  