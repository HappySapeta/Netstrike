#include "Engine/Engine.h"

#include "Tank.h"

std::unique_ptr<NS::Engine> NS::Engine::Instance_(nullptr);

NS::Engine::Engine()
	:Networking_(NS::Networking::Get())
{
	std::unique_ptr<Actor> TempActor = std::make_unique<Actor>();
	ActorConstructors_.insert({TempActor->GetTypeInfo(), std::move(TempActor)});
	
	std::unique_ptr<Tank> TempTank = std::make_unique<Tank>();
	ActorConstructors_.insert({TempTank->GetTypeInfo(), std::move(TempTank)});
}

NS::Engine* NS::Engine::Get()
{
	if (!Instance_)
	{
		Instance_ = std::unique_ptr<Engine>(new Engine());
	}
	
	return Instance_.get();
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

void NS::Engine::StartSubsystems()
{
	if (Networking_)
	{
#ifdef NS_SERVER
		Networking_->Server_Listen();
#endif
#ifdef NS_CLIENT
		Networking_->Client_ConnectToServer(NS::SERVER_ADDRESS, NS::SERVER_PORT);
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

NS::Actor* NS::Engine::CreateActor(const size_t TypeHash)
{
	Actors_.emplace_back(ActorConstructors_.at(TypeHash)->CreateCopy());
	Actor* NewActor = Actors_.back().get();
			
	std::vector<ReplicatedProp> ReplicatedProps;
	NewActor->GetReplicatedProperties(ReplicatedProps);
			
	if (Networking_)
	{
#ifdef NS_SERVER
		Networking_->Server_RegisterNewActor(NewActor);
#endif
		Networking_->AddReplicateProps(ReplicatedProps);
	}
			
	return NewActor;
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
