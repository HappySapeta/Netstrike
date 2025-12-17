#include "Tank.h"
#include "Projectile.h"
#include "Engine/Engine.h"
#ifdef NS_CLIENT
#include "Input.h"
#endif

#include "InputConfiguration.h"
#include "Actor/SpriteComponent.h"
#include "Networking/Networking-Macros.h"

constexpr float MOVEMENT_SPEED = 1.0f;
constexpr float BOT_MOVEMENT_SPEED = 1.0f;
constexpr float TURN_RATE = 1.0f;
constexpr float BOT_TURN_RATE = 1.0f;
constexpr float TURRET_TURN_RATE = 1.0f;
constexpr float PROJECTILE_SPEED = 1000.0f;
constexpr float TANK_HEALTH = 100.0f;
constexpr float MAX_POSITION_ERROR = 60;
constexpr float MIN_HEADING_DOT = 0.8f;
constexpr float FIRE_DELAY = 0.5f;

constexpr float Deg2Rad(const float Deg)
{
	return Deg / 57.2958f;
}
sf::Vector2f GetSafeNormal(const sf::Vector2f& Vector)
{
	const float Length = Vector.length();
	if (Length < 0.00001f)
	{
		return {0, 0};
	}
	return Vector / Length;
}

NS::Tank::Tank()
{
	Heading_ = {0, -1};
	TurretAngle_ = 0.0f;
	Health_ = TANK_HEALTH;
	LocalSimulatedPosition_ = {0, 0};
	LocalSimulatedHeading_ = {0, -1};
	LocalVelocity_ = {0, 0};
	PreviousPosition_ = {0, 0};
	WanderTheta_ = 0.0f;
	LastFiredTime = ChronoClock::now();
	InterpolatedPosition_ = {0, 0};
	InterpolatedHeading_ = {0, -1};
	
	BodySpriteComp_ = AddComponent<SpriteComponent>();
	BodySpriteComp_->SetTexture(TANK_TEXTURE);
	
	TurretSpriteComp_ = AddComponent<SpriteComponent>();
	TurretSpriteComp_->SetTexture(TURRET_TEXTURE);
}

NS::Actor* NS::Tank::CreateCopy()
{
	return new Tank();
}

void NS::Tank::InitInput(const bool bIsBot)
{
	bIsPlayerInputBound_ = true;
	bIsBot_ = bIsBot;
#ifdef NS_CLIENT
	if(!bIsBot_)
	{
		NS::Input* Input = NS::Input::Get();
		NS::Networking* Networking = NS::Networking::Get();
	
		auto MoveTankVertical = [this, Networking](const float Value) -> void
		{
			if (Value > 0)
			{
				LocalSimulatedPosition_ = LocalSimulatedPosition_ + Heading_ * MOVEMENT_SPEED;
				Networking->Client_CallRPC({this, "Server_MoveTankForward"});
			}
			else if (Value < 0)
			{
				LocalSimulatedPosition_ = LocalSimulatedPosition_ - Heading_ * MOVEMENT_SPEED;
				Networking->Client_CallRPC({this, "Server_MoveTankBackward"});
			}
		};
		Input->BindAxisVertical(MoveTankVertical);
	
		auto TurnTank = [this, Networking](const float Value)
		{
			if (Value > 0)
			{
				LocalSimulatedHeading_ = LocalSimulatedHeading_.rotatedBy(sf::degrees(TURN_RATE));
				Networking->Client_CallRPC({this, "Server_TurnRight"});
			}
			else if (Value < 0)
			{
				LocalSimulatedHeading_ = LocalSimulatedHeading_.rotatedBy(sf::degrees(-TURN_RATE));
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
			const ChronoDuration TimeSinceLastFired = ChronoClock::now() - LastFiredTime;
			if (TimeSinceLastFired.count() > FIRE_DELAY)
			{
				LastFiredTime = ChronoClock::now();
				Networking->Client_CallRPC({this, "Server_Fire"});
			}
		};
		Input->BindOnKeyPressed(NS::Fire, Fire);
	}
#endif
}

void NS::Tank::BotUpdate()
{
#ifdef NS_CLIENT
	NS::Networking::Get()->Client_CallRPC({this, "Server_BotMoveForward"});
		
	std::mt19937 Generator(RandomDevice());
	Generator.seed(NetId_);
	std::normal_distribution<float> Distribution(-1.0f, 1.0f);
		
	constexpr float WanderRadius = 500.0f;
	constexpr float WanderLookAhead = 300.0f;
	const sf::Vector2f WanderCentre = Position_ + Heading_.normalized() * WanderLookAhead;
	WanderTheta_ += Distribution(Generator);
		
	const sf::Vector2f RandPos = 
	{
		WanderCentre.x + (WanderRadius * std::cos(Deg2Rad(WanderTheta_))), 
		WanderCentre.y + (WanderRadius * std::sin(Deg2Rad(WanderTheta_)))
	};
		
	const sf::Vector2f TargetHeading = GetSafeNormal(RandPos - Position_);
	const float TargetHeadingAngle = TargetHeading.length() == 0 ? 0.0f : TargetHeading.angle().asDegrees();
	if (TargetHeadingAngle > 0)
	{
		NS::Networking::Get()->Client_CallRPC({this, "Server_BotTurnRight"});
	}
	else
	{
		NS::Networking::Get()->Client_CallRPC({this, "Server_BotTurnLeft"});
	}
#endif
}

void NS::Tank::Update(const float DeltaTime)
{
	Actor::Update(DeltaTime);
#ifdef NS_CLIENT
	const sf::Vector2f PredictedPosition = PerformInterpolation(DeltaTime);
	BodySpriteComp_->SetPosition(PredictedPosition);
	TurretSpriteComp_->SetPosition(PredictedPosition);
	
	if (bIsBot_)
	{
		BotUpdate();
	}
#endif
#ifdef NS_SERVER
	BodySpriteComp_->SetPosition(Position_);
	TurretSpriteComp_->SetPosition(Position_);
#endif
	
	const sf::Vector2f PredictedHeading = PerformHeadingInterpolation(DeltaTime);
	BodySpriteComp_->SetRotation(PredictedHeading.angle());
	TurretSpriteComp_->SetRotation(sf::degrees(TurretAngle_));
	
	LocalVelocity_ = (Position_ - PreviousPosition_) / DeltaTime;
	PreviousPosition_ = Position_;
}

sf::Vector2f NS::Tank::PerformInterpolation(float DeltaTime)
{
	InterpolatedPosition_ = Position_;
	if (bShouldDoInterpolation_)
	{
		// For player
		if (!bIsBot_ && bIsPlayerInputBound_)
		{
			const float Distance = (LocalSimulatedPosition_ - Position_).length();
			if (Distance >= MAX_POSITION_ERROR)
			{
				InterpolatedPosition_ = Position_;
			}
			else
			{
				InterpolatedPosition_ = LocalSimulatedPosition_;
			}
		}
		// For bots
		else
		{
			sf::Vector2f PredictedPosition = PreviousPosition_ + LocalVelocity_ * DeltaTime;
			InterpolatedPosition_ = PredictedPosition;
		}
	}
	
	LocalSimulatedPosition_ = InterpolatedPosition_;
	return InterpolatedPosition_;
}

sf::Vector2f NS::Tank::PerformHeadingInterpolation(float DeltaTime)
{
	InterpolatedHeading_ = Heading_;
	if (bShouldDoInterpolation_)
	{
		// For player
		if (!bIsBot_ && bIsPlayerInputBound_)
		{
			const float DotProduct = GetSafeNormal(Heading_).dot(GetSafeNormal(LocalSimulatedHeading_));
			if (DotProduct < MIN_HEADING_DOT)
			{
				InterpolatedHeading_ = Heading_;
			}
			else
			{
				InterpolatedHeading_ = LocalSimulatedHeading_;
			}
		}
		// For bots
		else
		{
			InterpolatedHeading_ = Heading_;
		}
	}
	
	LocalSimulatedHeading_ = InterpolatedHeading_;
	return InterpolatedHeading_;
}

size_t NS::Tank::GetTypeInfo() const
{
	return typeid(this).hash_code();
}

void NS::Tank::SetPosition(const sf::Vector2f& NewPosition)
{
	static bool IsPredictedPosInitialized = false;
	if (!IsPredictedPosInitialized)
	{
		LocalSimulatedPosition_ = NewPosition;
		IsPredictedPosInitialized = true;
	}
	
	Actor::SetPosition(NewPosition);
}

void NS::Tank::DoDamage(float Damage)
{
#ifdef NS_SERVER
	Health_ -= Damage;
	if (Health_ <= 0)
	{
		NS::Engine::Get()->DestroyActor(this);
	}
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

void NS::Tank::Server_BotMoveForward()
{
	SetPosition(GetPosition() + Heading_ * BOT_MOVEMENT_SPEED);
}

void NS::Tank::Server_BotTurnLeft()
{
	Heading_ = Heading_.rotatedBy(sf::degrees(-BOT_TURN_RATE));
}

void NS::Tank::Server_BotTurnRight()
{
	Heading_ = Heading_.rotatedBy(sf::degrees(BOT_TURN_RATE));
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
	Projectile* Projectile = NS::Engine::Get()->CreateActor<NS::Projectile>();
	
	sf::Vector2f LaunchVelocity{-sin(Deg2Rad(TurretAngle_ + 90.0f)), cos(Deg2Rad(TurretAngle_ + 90.0f))};
	LaunchVelocity = GetSafeNormal(LaunchVelocity) * PROJECTILE_SPEED;
	Projectile->SetPosition(GetPosition());
	Projectile->Launch(LaunchVelocity, this);
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
	REGISTER_RPC(Tank, Server_BotTurnLeft);
	REGISTER_RPC(Tank, Server_BotTurnRight);
	REGISTER_RPC(Tank, Server_BotMoveForward);
}