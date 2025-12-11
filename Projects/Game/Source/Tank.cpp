#include "Tank.h"
#include "Actor/SpriteComponent.h"

constexpr float MOVEMENT_SPEED = 2.0f;
constexpr float TURN_RATE = 2.0f;
const char* TANK_TEXTURE = "Textures\\Tank.png";

NS::Tank::Tank()
{
	Heading_ = {0, -1};
	
	SpriteComp_ = AddComponent<SpriteComponent>();
	if (SpriteComp_)
	{
		SpriteComp_->SetTexture(TANK_TEXTURE);
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

void NS::Tank::MoveForward()
{
#ifdef NS_CLIENT
	NS::Networking::Get()->Client_CallRPC({this, "Server_MoveTankForward"});
#endif
}

void NS::Tank::MoveBackward()
{
#ifdef NS_CLIENT
	NS::Networking::Get()->Client_CallRPC({this, "Server_MoveTankBackward"});
#endif
}

void NS::Tank::TurnRight()
{
#ifdef NS_CLIENT
	NS::Networking::Get()->Client_CallRPC({this, "Server_TurnRight"});
#endif
}

void NS::Tank::TurnLeft()
{
#ifdef NS_CLIENT
	NS::Networking::Get()->Client_CallRPC({this, "Server_TurnLeft"});
#endif
}

void NS::Tank::Server_MoveTankForward()
{
	SetPosition(GetPosition() + Heading_ * MOVEMENT_SPEED);
}

void NS::Tank::Server_MoveTankBackward()
{
	SetPosition(GetPosition() - Heading_ * MOVEMENT_SPEED);
}

void NS::Tank::Server_TurnLeft()
{
	Heading_ = Heading_.rotatedBy(sf::degrees(-TURN_RATE));
}

void NS::Tank::Server_TurnRight()
{
	Heading_ = Heading_.rotatedBy(sf::degrees(TURN_RATE));
}

void NS::Tank::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	OutReplicatedProperties.push_back({this, offsetof(Tank, Position_), sizeof(Position_)});
	OutReplicatedProperties.push_back({this, offsetof(Tank, Heading_), sizeof(Heading_)});
}

void NS::Tank::GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps)
{
	OutRpcProps.push_back({"Server_MoveTankForward", [](Actor* Actor)
	{
		if (Tank* TankPtr = dynamic_cast<Tank*>(Actor))
		{
			TankPtr->Server_MoveTankForward();
		}
	}});
	
	OutRpcProps.push_back({"Server_MoveTankBackward", [](Actor* Actor)
	{
		if (Tank* TankPtr = dynamic_cast<Tank*>(Actor))
		{
			TankPtr->Server_MoveTankBackward();
		}
	}});
	
	OutRpcProps.push_back({"Server_TurnLeft", [](Actor* Actor)
	{
		if (Tank* TankPtr = dynamic_cast<Tank*>(Actor))
		{
			TankPtr->Server_TurnLeft();
		}
	}});
	
	OutRpcProps.push_back({"Server_TurnRight", [](Actor* Actor)
	{
		if (Tank* TankPtr = dynamic_cast<Tank*>(Actor)) 
		{
			TankPtr->Server_TurnRight();
		}
	}});
}

void NS::Tank::Update(const float DeltaTime)
{
	Actor::Update(DeltaTime);
	SpriteComp_->SetRotation(Heading_.angle());
	SpriteComp_->SetPosition(Position_);
}
