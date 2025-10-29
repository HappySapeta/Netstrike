#pragma once
#include <memory>
#include <functional>
#include <optional>
#include <SFML/Window/Event.hpp>

namespace NS
{
	typedef std::function<void(const float InputValue)> InputAction;
	class Input
	{
	public:

		static Input* Get();
		void HandleAxes();
		void Update(const std::optional<sf::Event>& event);

	public:

		void BindAxisHorizontal(const InputAction& Callback);
		void UnBindAxisHorizontal();
		
		void BindAxisVertical(const InputAction& Callback);
		void UnBindAxisVertical();
		
	private:
		static std::unique_ptr<Input> Instance_;
		InputAction HorizontalCallback;
		InputAction VerticalCallback;
	};
}
