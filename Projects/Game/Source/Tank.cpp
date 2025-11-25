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

NS::Actor* NS::Tank::CreateCopy()
{
	return new Tank();
}

const char* NS::Tank::GetTypeInfo() const
{
	return "Tank";
}
