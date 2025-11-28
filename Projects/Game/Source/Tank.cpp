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

NS::Actor* NS::Tank::CreateCopy()
{
	return new Tank();
}

size_t NS::Tank::GetTypeInfo() const
{
	return typeid(this).hash_code();
}

void NS::Tank::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	OutReplicatedProperties.push_back({this, offsetof(Tank, Position_), sizeof(Position_)});
}
