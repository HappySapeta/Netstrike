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
		void InitInput();
		bool GetIsPlayerInputIntialized() const
		{
			return playerInputInitialized;
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
		void Update(const float DeltaTime) override;

	protected:
		
		sf::Vector2f Heading_; // REPLICATED
		float TurretAngle_; // REPLICATED
		float Health_;
		
		SpriteComponent* BodySpriteComp_ = nullptr;
		SpriteComponent* TurretSpriteComp_ = nullptr;
		const sf::RenderWindow* Window_ = nullptr;

	private:
		
		bool playerInputInitialized = false;
	};
}
