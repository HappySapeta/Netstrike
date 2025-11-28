#include "Actor/Actor.h"

#include "Logger.h"
#include "Actor/ActorComponent.h"

std::unique_ptr<NS::Actor> NS::Actor::StaticInstance_(nullptr);

NS::Transform::Transform()
	:Position({0,0}), Rotation({0,0}), Scale({1,1})
{}

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

void NS::Actor::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	OutReplicatedProperties.push_back({this, offsetof(Actor, TestVariable), sizeof(TestVariable)});
}
