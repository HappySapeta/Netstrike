#include "World.h"

NS::World::World()
{
	SpriteComp_ = AddComponent<SpriteComponent>();
	if (SpriteComp_)
	{
		SpriteComp_->SetTexture(NS::GRASS_TEXTURE);
		SpriteComp_->SetTextRect({{0, 0},{NS::WORLD_SIZE, NS::WORLD_SIZE}});
	}
}
