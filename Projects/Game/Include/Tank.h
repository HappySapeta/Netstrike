#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class Tank : public NS::Actor
	{
	public:
		Tank();
		
		[[nodiscard]] virtual Actor* CreateCopy() override;
		virtual void Update(const float DeltaTime) override;
		virtual size_t GetTypeInfo() const override;

	private:
		class SpriteComponent* SpriteComp_ = nullptr;
				
	private:
		
		static std::unique_ptr<Actor> StaticInstance_;
	};
}
