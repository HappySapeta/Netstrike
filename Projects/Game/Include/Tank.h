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
		void RPC_MoveRandom();
		
	protected:
		
		void MoveTank();

	private:

		void GetReplicatedProperties(std::vector<NS::ReplicatedProp>& OutReplicatedProperties) override;
		void GetRPCSignatures(std::vector<NS::RPCProp>& OutRpcProps) override;

		class SpriteComponent* SpriteComp_ = nullptr;
		static std::unique_ptr<Actor> StaticInstance_;
	};
}
