#pragma once
#include <cstdint>
#include <SFML/Network/IpAddress.hpp>

namespace NS
{
	constexpr uint16_t SCREEN_WIDTH = 1280;
	constexpr uint16_t SCREEN_HEIGHT = 1280;

	const sf::IpAddress SERVER_ADDRESS = sf::IpAddress(127,0,0,1);
	constexpr uint16_t SERVER_PORT = 56789;
	constexpr int64_t SERVER_TICK_DURATION_MS = 500;

	constexpr float DEFAULT_CONNECTION_TIMEOUT = 30.0f;
	constexpr uint16_t DEBUG_SERVER_MAX_CONNECTIONS = 1;
	constexpr uint16_t MAX_PACKET_SIZE = 100;
	constexpr uint16_t SERVER_SELECTOR_WAIT_TIME_MS = 20;
	constexpr uint16_t CLIENT_SELECTOR_WAIT_TIME_MS = 10;
}
