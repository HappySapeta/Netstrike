#include <chrono>
#include <thread>

#include "GameConfiguration.h"
#include "Engine/Engine.h"
#include <Networking/Networking.h>

#include "Test/ReplicationTest.h"

static NS::Engine Engine;

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;

int main()
{
	NS::ReplicationTest Test;
	Test.Variable = 25.2345f;
	
	NS::Networking* Networking = NS::Networking::Get();
	Networking->Server_Listen();

	Networking->Server_ReplicateToClient(&Test.Variable, sizeof(Test.Variable), 0, 0);
	Networking->Start();

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
	}
	
	Networking->Stop();
	return 0;
}  