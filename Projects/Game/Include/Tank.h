#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class Tank : public NS::Actor
	{
	public:
		Tank();
		
		virtual void Update(const float DeltaTime) override;
		virtual const char* GetTypeInfo() const override;
		Actor* CreateCopy() override;

	private:
		class SpriteComponent* SpriteComp_ = nullptr;
				
	private:
		
		static std::unique_ptr<Actor> StaticInstance_;
	};
}
