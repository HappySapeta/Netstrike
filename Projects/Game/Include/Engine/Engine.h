#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>

namespace NS
{
	class Actor;
}

namespace NS
{
	class Engine
	{
	public:
		Engine() = default;

		template<class ActorType, class... ParameterTypes>
		ActorType* CreateActor(ParameterTypes&&... Args)
		{
			Actors_.emplace_back(std::make_unique<ActorType>(std::forward<ParameterTypes>(Args)...));
			return static_cast<ActorType*>(Actors_.back().get());
		}

		void DestroyActor(Actor* ActorToDestroy);

		void Update(const float DeltaTime);
		void Draw(sf::RenderWindow& Window);

	private:

		std::vector<std::unique_ptr<Actor>> Actors_;
	};
}
