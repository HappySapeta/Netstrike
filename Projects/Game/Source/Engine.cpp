#include "Engine/Engine.h"

NS::Engine::Engine()
	:Networking_(NS::Networking::Get())
{}

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

void NS::Engine::StartSubsystems()
{
	if (Networking_)
	{
#ifdef NS_SERVER
		Networking_->Server_Listen();
#endif
#ifdef NS_CLIENT
		Networking_->TCPConnect(NS::SERVER_ADDRESS, NS::SERVER_PORT);
#endif
		
		Networking_->Start();
	}
}

void NS::Engine::StopSubsystems()
{
	if (Networking_)
	{
		Networking_->Stop();
	}
}

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
