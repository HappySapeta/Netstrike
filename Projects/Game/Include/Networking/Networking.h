#pragma once
#include <memory>
#include <SFML/Network.hpp>

namespace NS
{
	class Networking
	{
	public:

		static Networking* Get();

		Networking(const Networking&) = delete;
		Networking(Networking&&) = delete;
		Networking& operator=(const Networking&) = delete;
		Networking& operator=(Networking&&) = delete;

	public:
		
		void TCPConnect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
		void TCPListen();

	private:

		Networking() = default;

	private:

		static std::unique_ptr<Networking> Instance_;
		sf::TcpSocket ClientSocket_;

#ifdef NS_SERVER
		sf::TcpListener ServerSocket_;
		std::vector<sf::TcpSocket> PerClientSockets_;
#endif
	};
}
