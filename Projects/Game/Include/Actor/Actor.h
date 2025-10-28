#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>

#include "ActorComponent.h"

namespace NS
{
	struct Transform : sf::Transform
	{
		Transform();
		
		sf::Vector2f Position;
		sf::Vector2f Rotation;
		sf::Vector2f Scale;
	};

	class Actor
	{
	public:

		Actor() = default;
		virtual ~Actor() = default;

	public:

		template<class ComponentType, typename = std::enable_if_t<std::is_base_of_v<ActorComponent, ComponentType>>>
		ComponentType* AddComponent()
		{
			Components_.push_back(std::make_unique<ComponentType>());
			return static_cast<ComponentType*>(Components_.back().get());
		}

		virtual void Update(const float DeltaTime);

		void SetActorLocation(const sf::Vector2f NewLocation);
		const sf::Vector2f& GetActorLocation() const;

	protected:

		Transform Transform_;
		std::vector<std::unique_ptr<ActorComponent>> Components_;
	};
}
