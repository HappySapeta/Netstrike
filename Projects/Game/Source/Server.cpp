#include <chrono>
#include <thread>

#include "GameConfiguration.h"
#include "Tank.h"
#include "Engine/Engine.h"

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;

int main()
{
	NS::Engine* Engine = NS::Engine::Get();
	Engine->StartSubsystems();
	
	NS::Tank* Tank = Engine->CreateActor<NS::Tank>();
	Tank->SetPosition({100.0f, 100.0f});
	
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