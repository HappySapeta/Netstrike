#pragma once
#include <cstdint>
#include <SFML/Network/IpAddress.hpp>

namespace NS
{
	constexpr uint16_t SCREEN_WIDTH = 500;
	constexpr uint16_t SCREEN_HEIGHT = 500;
	constexpr uint16_t WORLD_SIZE = 2000;

	const sf::IpAddress SERVER_ADDRESS = sf::IpAddress(127,0,0,1);
	constexpr uint16_t SERVER_PORT = 56789;
	constexpr int64_t SERVER_TICK_DURATION_MS = 20;

	constexpr float DEFAULT_CONNECTION_TIMEOUT = 30.0f;
	constexpr uint16_t DEBUG_SERVER_MAX_CONNECTIONS = 10;
	constexpr uint16_t MAX_PACKET_SIZE = 10;
	constexpr uint16_t SERVER_SELECTOR_WAIT_TIME_MS = 20;
	constexpr uint16_t CLIENT_SELECTOR_WAIT_TIME_MS = 10;
	
	inline const char* TANK_TEXTURE = "Textures\\Tank.png";
	inline const char* GRASS_TEXTURE = "Textures\\Grass.png";
	inline const char* TURRET_TEXTURE = "Textures\\Turret.png";
	inline const char* PROJECTILE_TEXTURE = "Textures\\Projectile.png";
}
