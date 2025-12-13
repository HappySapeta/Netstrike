#include "Projectile.h"

#include "Engine/Engine.h"
#include "Networking/Networking-Macros.h"

constexpr float LIFE_TIME = 0.25f;

NS::Projectile::Projectile()
{
	Velocity_ = {0, 0};
	TimeAlive_ = 0.0f;
	
	SpriteComponent_ = AddComponent<SpriteComponent>();
	SpriteComponent_->SetTexture(PROJECTILE_TEXTURE);
}

NS::Projectile::~Projectile()
{
	NSLOG(ELogLevel::INFO, "Projectile destroyed.");
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
	Actor::Update(DeltaTime);
	
	if (Velocity_.length() > 0)
	{
		SpriteComponent_->SetPosition(Position_);
		SpriteComponent_->SetRotation(Velocity_.angle());
	}
	
#ifdef NS_SERVER
	Position_ = Position_ + Velocity_ * DeltaTime;
	TimeAlive_ += DeltaTime;
	if (TimeAlive_ >= LIFE_TIME)
	{
		NS::Engine::Get()->DestroyActor(this);
	}
#endif
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
