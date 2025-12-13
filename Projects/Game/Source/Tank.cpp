#include "Tank.h"

#ifdef NS_CLIENT
#include "Input.h"
#endif

#include "InputConfiguration.h"
#include "Actor/SpriteComponent.h"
#include "Networking/Networking-Macros.h"

constexpr float MOVEMENT_SPEED = 0.05f;
constexpr float TURN_RATE = 0.02f;
constexpr float TURRET_TURN_RATE = 0.05f;

NS::Tank::Tank()
{
	Heading_ = {0, -1};
	TurretAngle_ = 0.0f;
	
	BodySpriteComp_ = AddComponent<SpriteComponent>();
	BodySpriteComp_->SetTexture(TANK_TEXTURE);
	
	TurretSpriteComp_ = AddComponent<SpriteComponent>();
	TurretSpriteComp_->SetTexture(TURRET_TEXTURE);
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
	NS::Networking* Networking = NS::Networking::Get();
	
	auto MoveTankVertical = [this, Networking](const float Value) -> void
	{
		if (Value > 0)
		{
			Networking->Client_CallRPC({this, "Server_MoveTankForward"});
		}
		else if (Value < 0)
		{
			Networking->Client_CallRPC({this, "Server_MoveTankBackward"});
		}
	};
	Input->BindAxisVertical(MoveTankVertical);
	
	auto TurnTank = [this, Networking](const float Value)
	{
		if (Value > 0)
		{
			Networking->Client_CallRPC({this, "Server_TurnRight"});
		}
		else if (Value < 0)
		{
			Networking->Client_CallRPC({this, "Server_TurnLeft"});
		}	
	};
	Input->BindAxisHorizontal(TurnTank);
	
	auto TurnTurret = [this, Networking](const float Value)
	{
		if (Value > 0)
		{
			Networking->Client_CallRPC({this, "Server_TurnTurretClockwise"});
		}
		else if (Value < 0)
		{
			Networking->Client_CallRPC({this, "Server_TurnTurretAntiClockwise"});
		}
	};
	Input->BindTurretAxis(TurnTurret);
	
	auto Fire = [this, Networking](const sf::Keyboard::Scancode Scancode)
	{
		Networking->Client_CallRPC({this, "Server_Fire"});	
	};
	Input->BindOnKeyPressed(NS::Fire, Fire);
	
#endif
}

void NS::Tank::Update(const float DeltaTime)
{
	Actor::Update(DeltaTime);
	BodySpriteComp_->SetRotation(Heading_.angle());
	BodySpriteComp_->SetPosition(Position_);
	TurretSpriteComp_->SetPosition(Position_);
	TurretSpriteComp_->SetRotation(sf::degrees(TurretAngle_));
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

void NS::Tank::Server_TurnTurretClockwise()
{
	TurretAngle_ += TURRET_TURN_RATE;
}

void NS::Tank::Server_TurnTurretAntiClockwise()
{
	TurretAngle_ -= TURRET_TURN_RATE;
}

void NS::Tank::Server_Fire()
{
	NSLOG(ELogLevel::INFO, "Fired!");
}

void NS::Tank::GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties)
{
	DO_REP(Tank, Position_);
	DO_REP(Tank, Heading_);
	DO_REP(Tank, TurretAngle_);
}

void NS::Tank::GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps)
{
	REGISTER_RPC(Tank, Server_MoveTankBackward)
	REGISTER_RPC(Tank, Server_MoveTankForward)
	REGISTER_RPC(Tank, Server_TurnLeft)
	REGISTER_RPC(Tank, Server_TurnRight)
	REGISTER_RPC(Tank, Server_TurnTurretClockwise)
	REGISTER_RPC(Tank, Server_TurnTurretAntiClockwise)
	REGISTER_RPC(Tank, Server_Fire)
}
