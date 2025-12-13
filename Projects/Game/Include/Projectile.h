#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class Projectile : public NS::Actor
	{
		friend class NS::Engine;
	public:

		Projectile();
		
		size_t GetTypeInfo() const override;
		[[nodiscard]] Actor* CreateCopy() override;
		void Update(const float DeltaTime) override;
		void SetVelocity(const sf::Vector2f NewVelocity);

	private:

		void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties) override;

	private:
		
		sf::Vector2f Velocity_; // REPLICATED
		NS::SpriteComponent* SpriteComponent_ = nullptr;
	};
}
