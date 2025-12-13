#pragma once
#include "ActorComponent.h"

#include <memory>
#include <string>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Actor.h"

namespace NS
{
	class SpriteComponent : public ActorComponent
	{
	public:
		virtual void Update(const float DeltaTime) override {};
		void Draw(sf::RenderWindow& RenderWindow);
		void SetTexture(const std::string& TextureResource);
		void SetTextRect(const sf::IntRect& Rect);

		void SetRotation(const sf::Angle& Angle)
		{
			Rotation_ = Angle;
		}
		
		void SetPosition(const sf::Vector2f& NewPosition)
		{
			Position_ = NewPosition;
		}

	private:
		std::unique_ptr<sf::Sprite> Sprite_;
		std::unique_ptr<sf::Texture> Texture_;
		
		sf::Angle Rotation_;
		sf::Vector2f Position_;
	};
}
