#include "Projectile.h"

#include "Networking/Networking-Macros.h"

NS::Projectile::Projectile()
{
	Velocity_ = {0, 0};
	
	SpriteComponent_ = AddComponent<SpriteComponent>();
	SpriteComponent_->SetTexture(PROJECTILE_TEXTURE);
}

size_t NS::Projectile::GetTypeInfo() const
{
	return typeid(this).hash_code();
}

NS::Actor* NS::Projectile::CreateCopy()
{
	return new Projectile();
}

void NS::Projectile::Update(const float DeltaTime)
{
#ifdef NS_SERVER
	Position_ = Position_ + Velocity_ * DeltaTime;
#endif
	
	if (Velocity_.length() > 0)
	{
		SpriteComponent_->SetPosition(Position_);
		SpriteComponent_->SetRotation(Velocity_.angle());
	}
	Actor::Update(DeltaTime);
}

void NS::Projectile::SetVelocity(const sf::Vector2f NewVelocity)
{
	Velocity_ = NewVelocity;
}

void NS::Projectile::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	DO_REP(Projectile, Position_);
	DO_REP(Projectile, Velocity_);
}
