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
		
		Engine();
		void Update(const float DeltaTime);
		void Draw(sf::RenderWindow& Window);
		void StartSubsystems();
		void StopSubsystems();
		
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
		void DestroyActor(Actor* ActorToDestroy);

	private:

		std::vector<std::unique_ptr<Actor>> Actors_;
		NS::Networking* Networking_;
	};
}
