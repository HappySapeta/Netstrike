#pragma once
#include <memory>
#include <string>
#include "ActorComponent.h"
#include <SFML/Graphics/Texture.hpp>

namespace sf
{
	class RenderWindow;
	class Sprite;
}

namespace NS
{
	class SpriteComponent : public ActorComponent
	{
	public:
		virtual void Update(const float DeltaTime) override {};
		void Draw(sf::RenderWindow& RenderWindow);
		void SetTexture(const std::string& TextureResource);

	private:
		std::unique_ptr<sf::Sprite> Sprite_;
		std::unique_ptr<sf::Texture> Texture_;
	};
}
