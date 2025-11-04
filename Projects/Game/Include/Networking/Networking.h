#pragma once
#include <memory>
#include <SFML/Network.hpp>

namespace NS
{
	enum class ENetAuthority
	{
		SERVER,
		CLIENT
	};
	
	class Networking
	{
	public:

		[[nodiscard]] static Networking* Get();

		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;

	public:
		
		[[nodiscard]] sf::TcpSocket& TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
		[[nodiscard]] sf::TcpListener& TCPListen();
		std::vector<sf::TcpSocket>& GetClientSockets();

	private:

		Networking();

	private:

		const ENetAuthority NetIdentity_;
		static std::unique_ptr<Networking> Instance_;
		sf::TcpSocket ClientSocket_;
		sf::TcpListener ServerSocket_;
		std::vector<sf::TcpSocket> PerClientSockets_;
	};
}
