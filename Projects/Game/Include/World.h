#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class World : public NS::Actor
	{
	public:
		
		World();
		
	private:
		
		class SpriteComponent* SpriteComp_ = nullptr;
			
	};
}
