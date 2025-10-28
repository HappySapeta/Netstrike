#include "Actor/SpriteComponent.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Logger.h"
#include "Utils.h"

void NS::SpriteComponent::Draw(sf::RenderWindow& RenderWindow)
{
	if (Sprite_)
	{
		RenderWindow.draw(*Sprite_);
	}
}

void NS::SpriteComponent::SetTexture(const std::string& TextureResource)
{
	const auto Path = GetResourcePath(TextureResource);
	if (!Texture_)
	{
		Texture_ = std::make_unique<sf::Texture>(Path);
	}
	else
	{
		if (!Texture_->loadFromFile(Path))
		{
			NSLOG(ELogLevel::ERROR, "Failed to load texture. {}", TextureResource);
			return;
		}
	}
	
	if (!Sprite_)
	{
		Sprite_ = std::make_unique<sf::Sprite>(*Texture_);
	}
	else
	{
		Sprite_->setTexture(*Texture_);
	}
}
