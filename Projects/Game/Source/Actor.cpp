#include "Actor/Actor.h"

#include "Logger.h"
#include "Actor/ActorComponent.h"

void NS::Actor::SetPosition(const sf::Vector2f& NewPosition)
{
	Position_ = NewPosition;			
}

sf::Vector2f NS::Actor::GetPosition() const
{
	return Position_;
}

void NS::Actor::Update(const float DeltaTime)
{
	for (const auto& Component : Components_)
	{
		Component->Update(DeltaTime);
	}
}

size_t NS::Actor::GetTypeInfo() const
{
	return typeid(this).hash_code();
}

NS::Actor* NS::Actor::CreateCopy()
{
	return new Actor();
}
