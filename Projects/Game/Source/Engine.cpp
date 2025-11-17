#include "Engine/Engine.h"

void NS::Engine::DestroyActor(Actor* ActorToDestroy)
{
	std::vector<std::unique_ptr<Actor>>::iterator Iterator = Actors_.begin();
	while (Iterator != Actors_.end())
	{
		Actor* Ptr = Iterator->get();
		if (Ptr == ActorToDestroy)
		{
			Iterator = Actors_.erase(Iterator);
		}
		else
		{
			++Iterator;
		}
	}
}

void NS::Engine::Update(const float DeltaTime)
{
	for (auto& Actor : Actors_)
	{
		Actor->Update(DeltaTime);
	}
}

void NS::Engine::Draw(sf::RenderWindow& Window)
{
	for (auto& Actor : Actors_)
	{
		std::vector<SpriteComponent*> SpriteComponents = Actor->GetComponents<SpriteComponent>();
		for (SpriteComponent* Ptr : SpriteComponents)
		{
			Ptr->Draw(Window);
		}
	}
}
