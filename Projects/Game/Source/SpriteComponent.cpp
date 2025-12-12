#include "Actor/SpriteComponent.h"
#include "Logger.h"
#include "Utils.h"
#include "Actor/Actor.h"

void NS::SpriteComponent::Draw(sf::RenderWindow& RenderWindow)
{
	if (Sprite_)
	{
		Sprite_->setOrigin((sf::Vector2f)Texture_->getSize() / 2.0f);
		Sprite_->setPosition(Position_);
		Sprite_->setRotation(Rotation_ + sf::degrees(-90));
		RenderWindow.draw(*Sprite_);
	}
}

void NS::SpriteComponent::SetTexture(const std::string& TextureResource)
{
	const auto Path = GetResourcePath(TextureResource);
	if (!Texture_)
	{
		Texture_ = std::make_unique<sf::Texture>(Path);
		Texture_->setRepeated(true);
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

void NS::SpriteComponent::SetTextRect(const sf::IntRect& Rect)
{
	Sprite_->setTextureRect(Rect);
}
