#pragma once
#include "Actor/Actor.h"

namespace NS
{
	class Tank : public NS::Actor
	{
	public:
		
		Tank();
		
		[[nodiscard]] virtual Actor* CreateCopy() override;
		virtual size_t GetTypeInfo() const override;
		void MoveForward();
		void MoveBackward();
		void TurnRight();
		void TurnLeft();

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
		
		sf::Vector2f Heading_;
		
	private:
		
		class SpriteComponent* SpriteComp_ = nullptr;
		static std::unique_ptr<Actor> StaticInstance_;
	};
}
