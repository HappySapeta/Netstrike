#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Actor/Actor.h"
#include "Actor/SpriteComponent.h"

namespace NS
{
	class Tank : public NS::Actor
	{
	public:
		Tank();
		virtual void Update(const float DeltaTime) override;
		virtual void Draw(sf::RenderWindow& Window);

	private:

		SpriteComponent* SpriteComp_ = nullptr;
	};
}
