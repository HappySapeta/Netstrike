#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class Tank : public NS::Actor
	{
	public:
		Tank();
		virtual void Update(const float DeltaTime) override;
		virtual void Draw(sf::RenderWindow& Window);

	private:
		class SpriteComponent* SpriteComp_ = nullptr;
	};
}
