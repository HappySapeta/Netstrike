#include <chrono>
#include <thread>
#include <random>

#include "GameConfiguration.h"
#include "Tank.h"
#include "Engine/Engine.h"

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;

sf::Vector2f GetRandomPosition()
{
	std::random_device Device;
	std::mt19937 Engine(Device());
	std::uniform_int_distribution<int> DistributionX(0, NS::SCREEN_WIDTH);
	std::uniform_int_distribution<int> DistributionY(0, NS::SCREEN_HEIGHT);
	
	return {static_cast<float>(DistributionX(Engine)), static_cast<float>(DistributionY(Engine))};
}

void OnClientConnected(const NS::NetClient* NewClient)
{
	NSLOG(NS::ELogLevel::INFO, "New client connected : {}", NewClient->ClientId);
	NS::Tank* NewTank = NS::Engine::Get()->CreateActor<NS::Tank>(NewClient->ClientId);
	
	NewTank->SetPosition(GetRandomPosition());
}

int main()
{
	NS::Engine* Engine = NS::Engine::Get();
	NS::Networking::Get()->Server_AssignOnClientConnected(&OnClientConnected);
	Engine->StartSubsystems();
	
	while (true)
	{
		static TimePoint TickStart;
		TickStart = HRClock::now();
		
		Engine->Update(0.016f);

		Duration TickDuration = HRClock::now() - TickStart;
		DeltaTimeSecs = TickDuration.count();

		const auto TickDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(TickDuration);
		const auto TimeToWaitMs = NS::SERVER_TICK_DURATION_MS - TickDurationMs.count();
		if (TimeToWaitMs > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(TimeToWaitMs));
		}
	}
	
	Engine->StopSubsystems();
	return 0;
}  