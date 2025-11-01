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

#ifdef NS_CLIENT
		void Connect(const sf::IpAddress& ServerAddress, const uint16_t ServerPort);
#endif

#ifdef NS_SERVER
		void Listen();
#endif

	private:

		Networking() = default;

	private:

		static std::unique_ptr<Networking> Instance_;

#ifdef NS_CLIENT
		sf::TcpSocket ClientSocket_;
#endif

#ifdef NS_SERVER
		sf::TcpListener ServerSocket_;
		std::vector<sf::TcpSocket> PerClientSockets_;
#endif
	};
}
