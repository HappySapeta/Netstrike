#include "Tank.h"

#include "Logger.h"
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

size_t NS::Tank::GetTypeInfo() const
{
	return typeid(this).hash_code();
}
