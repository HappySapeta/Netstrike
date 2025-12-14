#pragma once
#include "Actor/Actor.h"

class SpriteComponent;

namespace NS
{
	class Tank : public NS::Actor
	{
		friend class NS::Engine;
	
	public:
		
		Tank();
		
		[[nodiscard]] virtual Actor* CreateCopy() override;
		virtual size_t GetTypeInfo() const override;
		virtual void SetPosition(const sf::Vector2f& NewPosition) override;
		void InitInput();
		bool GetIsInputInitalized() const
		{
			return IsPlayerInputBound_;
		}

		void DoDamage(float Damage);

	protected:
		
		void Server_MoveTankForward();
		void Server_MoveTankBackward();
		void Server_TurnLeft();
		void Server_TurnRight();
		void Server_TurnTurretClockwise();
		void Server_TurnTurretAntiClockwise();
		void Server_Fire();

	private:

		void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties) override;
		void GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps) override;
		sf::Vector2f PerformInterpolation(float DeltaTime);
		void Update(const float DeltaTime) override;

	protected:
		
		sf::Vector2f LocalSimulatedPosition_;
		sf::Vector2f LocalVelocity_;
		sf::Vector2f PreviousPosition_;
		sf::Vector2f Heading_; // REPLICATED
		float TurretAngle_; // REPLICATED
		float Health_;
		
		SpriteComponent* BodySpriteComp_ = nullptr;
		SpriteComponent* TurretSpriteComp_ = nullptr;
		const sf::RenderWindow* Window_ = nullptr;

	private:
		
		bool IsPlayerInputBound_ = false;
	};
}
