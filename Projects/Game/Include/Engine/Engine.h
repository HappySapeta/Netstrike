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
		
#pragma region DELETED METHODS
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
#pragma endregion
		
	public:

		template<class ActorType, class... ParameterTypes>
		ActorType* CreateActor(ParameterTypes&&... Args)
		{
			Actors_.emplace_back(std::make_unique<ActorType>(std::forward<ParameterTypes>(Args)...));
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
			
			return static_cast<ActorType*>(NewActor);
		}
		Actor* CreateActor(const size_t TypeHash);
		void DestroyActor(Actor* ActorToDestroy);
		
	private:
		
		Engine();

	private:

		static std::unique_ptr<Engine> Instance_;
		std::vector<std::unique_ptr<Actor>> Actors_;
		NS::Networking* Networking_;
		std::unordered_map<size_t, std::unique_ptr<Actor>> ActorConstructors_;
	};
}
