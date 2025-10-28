#pragma once

namespace NS
{
	class ActorComponent
	{
	public:
		virtual ~ActorComponent() = default;
		
	public:
		virtual void Update(const float DeltaTime) {};
	};
}
