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

		void Connect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
		void Listen();

	private:

		Networking() = default;

	private:

		static std::unique_ptr<Networking> Instance_;
		sf::TcpSocket MainSocket_;
		sf::TcpListener MainListener_;
		std::vector<sf::TcpSocket> ClientSockets_;
	};
}
