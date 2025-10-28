#include "Tank.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Window.hpp>

#include "Logger.h"

NS::Tank::Tank()
{
	Debug_SetTexture("../../Resources/Tank.png");
}

void NS::Tank::Draw(sf::RenderWindow& Window)
{
	sf::Sprite Sprite(Texture_);
	Window.draw(Sprite);
}

void NS::Tank::Update(const float DeltaTime)
{
}

void NS::Tank::Debug_SetTexture(const char* TextureLocation)
{
	if (!Texture_.loadFromFile(TextureLocation))
	{
		NSLOG(ELogLevel::ERROR, "Failed to find texture. ", TextureLocation);
	}
}
