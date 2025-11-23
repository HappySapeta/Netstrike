#include "Actor/Actor.h"
#include "Actor/ActorComponent.h"

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

void NS::Actor::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	OutReplicatedProperties.push_back({this, offsetof(Actor, TestVariable), sizeof(TestVariable)});
}
