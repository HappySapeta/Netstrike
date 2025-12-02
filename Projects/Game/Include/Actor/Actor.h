#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "ActorComponent.h"
#include "Logger.h"
#include "SpriteComponent.h"
#include "Networking/Networking.h"

namespace NS
{
	class Actor
	{
		friend class Engine;
	public:

		Actor() = default;
		virtual ~Actor() = default;

	public:
		
		void SetPosition(const sf::Vector2f& NewPosition)
		{
			Num_ = 10;
			Position_ = NewPosition;			
		}
		
		sf::Vector2f GetPosition() const
		{
			return Position_;
		}
		
		void TestSomething()
		{
			NSLOG(ELogLevel::INFO, "TestSomething RPC invoked! {}", reinterpret_cast<uint64_t>(this));
		}
		
	public:

		template<class ComponentType, typename = std::enable_if_t<std::is_base_of_v<ActorComponent, ComponentType>>>
		ComponentType* AddComponent()
		{
			Components_.push_back(std::make_unique<ComponentType>());
			Components_.back()->Attach(this);
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

		virtual size_t GetTypeInfo() const;
		[[nodiscard]] virtual Actor* CreateCopy();

	private:
		
		virtual void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties);
		virtual void GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps);
		virtual void Update(const float DeltaTime);

	protected:

		uint8_t Num_ = 0;
		sf::Vector2f Position_ = {0.0f, 0.0f};
		float TestVariable = 0.0f;
		std::vector<std::unique_ptr<ActorComponent>> Components_;
		
	private:
		
		static std::unique_ptr<Actor> StaticInstance_;
	};
}
