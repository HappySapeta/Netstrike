#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>

namespace NS
{
	struct Transform : sf::Transform
	{
		Transform();
		
		sf::Vector2f Position;
		sf::Vector2f Rotation;
		sf::Vector2f Scale;
	};

	class Actor
	{
	public:

		Actor() = default;
		virtual ~Actor() = default;

	public:

		virtual void Draw(sf::RenderWindow& Window) = 0;
		virtual void Update(const float DeltaTime) = 0;

	protected:

		Transform Transform_;
	};
}
