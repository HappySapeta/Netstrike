#pragma once
#include <memory>
#include <functional>
#include <optional>
#include <SFML/Window/Event.hpp>

namespace NS
{
	typedef std::function<void(const float InputValue)> InputAxisBinding;
	typedef std::function<void(const sf::Keyboard::Scancode Scancode)> InputActionBinding;
	class Input
	{
	public:

		static Input* Get();
		void Update(const std::optional<sf::Event>& event);

	public:

		void BindOnKeyPressed(sf::Keyboard::Scancode Key, const InputActionBinding& Callback);
		void UnBindOnKeyPressed(sf::Keyboard::Scancode Key);
		void BindOnKeyReleased(sf::Keyboard::Scancode Key, const InputActionBinding& Callback);
		void UnBindOnKeyReleased(sf::Keyboard::Scancode Key);

		void BindAxisHorizontal(const InputAxisBinding& Callback);
		void UnBindAxisHorizontal();
		
		void BindAxisVertical(const InputAxisBinding& Callback);
		void UnBindAxisVertical();

	private:

		void HandleAxes();
		void HandleEvent(const std::optional<sf::Event>& Event);
		
	private:
		static std::unique_ptr<Input> Instance_;
		InputAxisBinding HorizontalCallback;
		InputAxisBinding VerticalCallback;
		std::array<InputActionBinding, sf::Keyboard::ScancodeCount> PressedCallbacks;
		std::array<InputActionBinding, sf::Keyboard::ScancodeCount> ReleasedCallbacks;
	};
}
