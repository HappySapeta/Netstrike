#include <chrono>
#include <thread>
#include "GameConfiguration.h"
#include "Logger.h"
#include "Tank.h"
#include "Engine/Engine.h"

static NS::Engine Engine;
static NS::Tank* Tank = nullptr;

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;

int main()
{
	Tank = Engine.CreateActor<NS::Tank>();
	Tank->SetActorLocation({NS::SCREEN_WIDTH * 0.5f, NS::SCREEN_HEIGHT * 0.5f});
	
	while (true)
	{
		static TimePoint TickStart;
		TickStart = HRClock::now();
		
		Engine.Update(0.016f);
		
		Duration TickDuration = HRClock::now() - TickStart;
		DeltaTimeSecs = TickDuration.count();

		const auto TickDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(TickDuration);
		const auto TimeToWaitMs = NS::SERVER_TICK_DURATION_MS - TickDurationMs.count();
		if (TimeToWaitMs > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(TimeToWaitMs));
		}

		Duration DelayedDuration = HRClock::now() - TickStart;
		NSLOG(NS::ELogLevel::INFO, "DeltaTime | Actual = {}, Delayed = {}.", TickDuration.count(), DelayedDuration.count());
	}

	return 0;
}  