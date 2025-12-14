#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Actor/Actor.h"

namespace NS
{
	class Engine
	{
	public:
		
		[[nodiscard]] static Engine* Get();
		void Update(const float DeltaTime);
		void Draw(sf::RenderWindow& Window);
		void StartSubsystems();
		void StopSubsystems();
		std::vector<NS::Actor*> GetActors() const;

#pragma region DELETED METHODS
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
#pragma endregion
		
	public:

		template<class ActorType, class... ParameterTypes>
		ActorType* CreateActor(IdentifierType AuthNetId = -1, ParameterTypes&&... Args)
		{
			Actors_.emplace_back(std::make_unique<ActorType>(std::forward<ParameterTypes>(Args)...));
			Actor* NewActor = Actors_.back().get();
			
			std::vector<ReplicatedProp> ReplicatedProps;
			NewActor->GetReplicatedProperties(ReplicatedProps);
			
			std::vector<RPCProp> RpcProps;
			NewActor->GetRPCSignatures(RpcProps);
			
			if (Networking_)
			{
#ifdef NS_SERVER
				Networking_->Server_RegisterNewActor(NewActor, AuthNetId);
#endif
				Networking_->AddReplicateProps(ReplicatedProps);
				Networking_->AddRPCProps(RpcProps);
			}
			
			return static_cast<ActorType*>(NewActor);
		}
		Actor* CreateActor(const size_t TypeHash);
		void DestroyActor(Actor* ActorToDestroy);

#ifdef NS_CLIENT
		template<class ActorType>
		ActorType* GetOwnedActor()
		{
			for (const auto& Actor : Actors_)
			{
				if (Actor->GetNetId() == NS::Networking::Get()->Client_GetNetId())
				{
					if (ActorType* Ptr = dynamic_cast<ActorType*>(Actor.get()))
					{
						return Ptr;
					}
				}
			}
			
			return nullptr;
		}
#endif
		
	private:
		
		Engine();
		void ClearGarbage();

	private:

		static std::unique_ptr<Engine> Instance_;
		std::vector<std::unique_ptr<Actor>> Actors_;
		NS::Networking* Networking_;
		std::unordered_map<size_t, std::unique_ptr<Actor>> ActorConstructors_;
	};
}
