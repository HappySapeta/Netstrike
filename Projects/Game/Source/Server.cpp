#include <chrono>
#include <thread>

#include "GameConfiguration.h"
#include "Engine/Engine.h"
#include <Networking/Networking.h>

static NS::Engine Engine;

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;

int main()
{
	NS::Networking* Networking = NS::Networking::Get();
	Networking->TCPListen();

	char Data[NS::PACKET_SIZE];
	const char* Message = "Hello World!";
	strncpy_s(Data, NS::PACKET_SIZE, Message, NS::PACKET_SIZE);

	NS::NetRequest Request;
	{
		Request.NetSource = NS::ENetAuthority::SERVER;
		Request.Reliability = NS::EReliability::RELIABLE;
		Request.InstructionType = NS::EInstructionType::REPLICATION;
		Request.Size = NS::PACKET_SIZE;
		Request.InstanceId = 0;
		Request.ObjectId = 0;

		memcpy_s(Request.Data, NS::PACKET_SIZE, Data, NS::PACKET_SIZE);
	}

	Networking->PushRequest(Request);

	while (true)
	{
		static TimePoint TickStart;
		TickStart = HRClock::now();
		
		Engine.Update(0.016f);
		Networking->Update();


		Duration TickDuration = HRClock::now() - TickStart;
		DeltaTimeSecs = TickDuration.count();

		const auto TickDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(TickDuration);
		const auto TimeToWaitMs = NS::SERVER_TICK_DURATION_MS - TickDurationMs.count();
		if (TimeToWaitMs > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(TimeToWaitMs));
		}
	}

	return 0;
}  