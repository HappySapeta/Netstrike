#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class Tank;

	class Projectile : public NS::Actor
	{
		friend class NS::Engine;
	public:

		Projectile();
		
		size_t GetTypeInfo() const override;
		[[nodiscard]] Actor* CreateCopy() override;
		void Update(const float DeltaTime) override;
		void Launch(sf::Vector2f NewVelocity, const Tank* ParentTank);
		const Tank* GetParentTank() const
		{
			return ParentTank_;
		}

	private:

		void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties) override;

	private:
		
		const NS::Tank* ParentTank_ = nullptr;
		sf::Vector2f Velocity_; // REPLICATED
		NS::SpriteComponent* SpriteComponent_ = nullptr;
		float TimeAlive_;
	};
}
