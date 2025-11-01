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

	private:
		std::unique_ptr<sf::Sprite> Sprite_;
		std::unique_ptr<sf::Texture> Texture_;
	};
}
