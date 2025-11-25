#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "ActorComponent.h"
#include "SpriteComponent.h"
#include "Networking/Networking.h"

namespace NS
{
	struct Transform
	{
		Transform();
		
		sf::Vector2f Position;
		sf::Vector2f Rotation;
		sf::Vector2f Scale;
	};

	class Actor
	{
		friend class Engine;
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

		template<class ComponentType, typename = std::enable_if_t<std::is_base_of_v<ActorComponent, ComponentType>>>
		std::vector<ComponentType*> GetComponents()
		{
			std::vector<ComponentType*> FoundComponents;
			for (auto& Component : Components_)
			{
				ActorComponent* Ptr = Component.get();
				if (dynamic_cast<ComponentType*>(Ptr))
				{
					FoundComponents.push_back(static_cast<ComponentType*>(Ptr));
				}
			}

			return FoundComponents;
		}

		virtual const char* GetTypeInfo() const;
		virtual Actor* CreateCopy();

	private:
		
		virtual void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties);
		virtual void Update(const float DeltaTime);

	protected:

		float TestVariable = 0.0f;
		Transform Transform_;
		std::vector<std::unique_ptr<ActorComponent>> Components_;
		
	private:
		
		static std::unique_ptr<Actor> StaticInstance_;
	};
}
