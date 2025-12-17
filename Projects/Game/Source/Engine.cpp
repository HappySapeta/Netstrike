#include "Engine/Engine.h"
#include "Projectile.h"
#include "Tank.h"

#define REGISTER_ACTOR(ActorType) \
{ \
std::unique_ptr<ActorType> TempActor = std::make_unique<ActorType>(); \
ActorConstructors_.insert({TempActor->GetTypeInfo(), std::move(TempActor)}); \
} \

std::unique_ptr<NS::Engine> NS::Engine::Instance_(nullptr);

NS::Engine::Engine()
	:Networking_(NS::Networking::Get())
{
	REGISTER_ACTOR(Actor);
	REGISTER_ACTOR(Tank);
	REGISTER_ACTOR(Projectile);
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
	ClearGarbage();
	
	for (auto& Actor : Actors_)
	{
		if (!Actor)
		{
			continue;
		}
		Actor->Update(DeltaTime);
	}
	
	if (Networking_->HasStarted())
	{
		Networking_->UpdateMainThread();
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
		Networking_->Client_ConnectToServer();
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

std::vector<NS::Actor*> NS::Engine::GetActors() const
{
	std::vector<NS::Actor*> Ptrs;
	for (const auto& Actor : Actors_)
	{
		Ptrs.push_back(Actor.get());
	}
	
	return Ptrs;
}

NS::Actor* NS::Engine::CreateActor(const size_t TypeHash)
{
	Actors_.emplace_back(ActorConstructors_.at(TypeHash)->CreateCopy());
	Actor* NewActor = Actors_.back().get();
			
	std::vector<ReplicatedProp> ReplicatedProps;
	NewActor->GetReplicatedProperties(ReplicatedProps);
	
	std::vector<RPCProp> RpcProps;
	NewActor->GetRPCSignatures(RpcProps);
			
	if (Networking_)
	{
		Networking_->AddReplicateProps(ReplicatedProps);
		Networking_->AddRPCProps(RpcProps);
	}
			
	OnActorCreated_(NewActor);
	return NewActor;
}

void NS::Engine::DestroyActor(Actor* ActorToDestroy)
{
#ifdef NS_SERVER
	Networking_->Server_DeRegisterActor(ActorToDestroy);
#endif
	ActorToDestroy->SetIsPendingKill(true);
}

void NS::Engine::ClearGarbage()
{
	std::vector<std::unique_ptr<Actor>>::iterator Iterator = Actors_.begin();
	while (Iterator != Actors_.end())
	{
		if (Iterator->get()->IsPendingKill())
		{
			Iterator = Actors_.erase(Iterator);
		}
		else
		{
			++Iterator;
		}
	}
}
