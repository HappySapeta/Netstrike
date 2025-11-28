#pragma once

namespace NS
{
	class Actor;
	
	class ActorComponent
	{
	public:
		virtual ~ActorComponent() = default;
		void Attach(Actor* Attachment)
		{
			Parent = Attachment;
		}
		
	public:
		virtual void Update(const float DeltaTime) {};
		
	protected:
		Actor* Parent = nullptr;
	};
}
