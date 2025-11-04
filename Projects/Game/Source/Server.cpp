#include <chrono>
#include <thread>

#include "GameConfiguration.h"
#include "Logger.h"
#include "Engine/Engine.h"
#include "Networking/Networking.h"

static NS::Engine Engine;

using HRClock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<float>;

static float DeltaTimeSecs = 0.016f;

int main()
{
	NS::Networking* Networking = NS::Networking::Get();
 	sf::TcpListener& Listener = Networking->TCPListen();
	std::vector<sf::TcpSocket>& ClientSockets = Networking->GetClientSockets();
	
	while (true)
	{
		static TimePoint TickStart;
		TickStart = HRClock::now();

		for (sf::TcpSocket& ClientSocket : ClientSockets)
		{
			sf::Packet Packet;
			const auto ReceiveStatus = ClientSocket.receive(Packet);
			if (ReceiveStatus != sf::Socket::Status::Done)
			{
				NSLOG(NS::ELogLevel::ERROR, "Failed to received data from client socket.",
					  ClientSocket.getRemoteAddress()->toString(), ClientSocket.getRemotePort());
				continue;
			}

			int Value;
			Packet >> Value; 
			NSLOG(NS::ELogLevel::INFO, "Received message : {}.", Value);
		}
		
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

	return 0;
}  