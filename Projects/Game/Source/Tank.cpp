#include "Tank.h"
#include "Actor/SpriteComponent.h"

NS::Tank::Tank()
{
	SpriteComp_ = AddComponent<SpriteComponent>();
	if (SpriteComp_)
	{
		SpriteComp_->SetTexture("Textures\\Tank.png");
	}
}

void NS::Tank::Update(const float DeltaTime)
{
}

void NS::Tank::Draw(sf::RenderWindow& Window)
{
	if (SpriteComp_)
	{
		SpriteComp_->Draw(Window);
	}
}
