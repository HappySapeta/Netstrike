#include "Tank.h"

#ifdef NS_CLIENT
#include "Input.h"
#endif

#include "Actor/SpriteComponent.h"

constexpr float MOVEMENT_SPEED = 0.05f;
constexpr float TURN_RATE = 0.02f;
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

void NS::Tank::InitInput()
{
	playerInputInitialized = true;
#ifdef NS_CLIENT
	NS::Input* Input = NS::Input::Get();
	auto MoveTankVertical = [this](const float Value) -> void
	{
		if (Value > 0)
		{
			NS::Networking::Get()->Client_CallRPC({this, "Server_MoveTankForward"});
		}
		else if (Value < 0)
		{
			NS::Networking::Get()->Client_CallRPC({this, "Server_MoveTankBackward"});
		}
	};
	Input->BindAxisVertical(MoveTankVertical);
	
	auto TurnTank = [this](const float Value)
	{
		if (Value > 0)
		{
			NS::Networking::Get()->Client_CallRPC({this, "Server_TurnRight"});
		}
		else if (Value < 0)
		{
			NS::Networking::Get()->Client_CallRPC({this, "Server_TurnLeft"});
		}	
	};
	Input->BindAxisHorizontal(TurnTank);
#endif
}

void NS::Tank::Update(const float DeltaTime)
{
	Actor::Update(DeltaTime);
	SpriteComp_->SetRotation(Heading_.angle());
	SpriteComp_->SetPosition(Position_);
}

size_t NS::Tank::GetTypeInfo() const
{
	return typeid(this).hash_code();
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
