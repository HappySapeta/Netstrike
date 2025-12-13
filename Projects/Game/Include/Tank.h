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

		void SetWindow(const sf::RenderWindow& Window);

	protected:
		
		void Server_MoveTankForward();
		void Server_MoveTankBackward();
		void Server_TurnLeft();
		void Server_TurnRight();

	private:

		void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties) override;
		void GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps) override;
		void Update(const float DeltaTime) override;

	protected:
		
		sf::Vector2f Heading_; // REPLICATED
		sf::Vector2f TurretHeading_;
		
		SpriteComponent* BodySpriteComp_ = nullptr;
		SpriteComponent* TurretSpriteComp_ = nullptr;
		const sf::RenderWindow* Window_ = nullptr;

	private:
		
		bool playerInputInitialized = false;
	};
}
