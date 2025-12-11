#include "Tank.h"

#include <random>

#include "Logger.h"
#include "Actor/SpriteComponent.h"

NS::Tank::Tank()
{
	SpriteComp_ = AddComponent<SpriteComponent>();
	if (SpriteComp_)
	{
		SpriteComp_->SetTexture("Textures\\Tank.png");
	}
}

NS::Actor* NS::Tank::CreateCopy()
{
	return new Tank();
}

size_t NS::Tank::GetTypeInfo() const
{
	return typeid(this).hash_code();
}

void NS::Tank::RPC_MoveRandom()
{
#ifdef NS_CLIENT
	NS::Networking::Get()->Client_CallRPC({this, "MoveTank"});
#endif
}

static sf::Vector2f GetRandomPosition()
{
	std::random_device Device;
	std::mt19937 Engine(Device());
	std::uniform_int_distribution<int> DistributionX(0, NS::SCREEN_WIDTH);
	std::uniform_int_distribution<int> DistributionY(0, NS::SCREEN_HEIGHT);
	
	return {static_cast<float>(DistributionX(Engine)), static_cast<float>(DistributionY(Engine))};
}

void NS::Tank::MoveTank()
{
#ifdef NS_SERVER
	SetPosition(GetRandomPosition());
#endif
}

void NS::Tank::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	OutReplicatedProperties.push_back({this, offsetof(Tank, Position_), sizeof(Position_)});
}

void NS::Tank::GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps)
{
	OutRpcProps.push_back({"MoveTank", [](Actor* Actor)
	{
		if (Tank* TankPtr = dynamic_cast<Tank*>(Actor))
		{
			TankPtr->MoveTank();
		}
	}});
}
