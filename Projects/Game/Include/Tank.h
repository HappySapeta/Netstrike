#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Actor/Actor.h"

namespace NS
{
	class Tank : public NS::Actor
	{
	public:

		Tank();
		
		virtual void Draw(sf::RenderWindow& Window) override;
		virtual void Update(const float DeltaTime) override;

	private:

		void Debug_SetTexture(const char* TextureLocation);
		
	private:
		sf::Texture Texture_;
	};
}
