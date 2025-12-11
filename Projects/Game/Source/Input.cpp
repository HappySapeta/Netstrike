#include "Input.h"

#include <optional>
#include <SFML/Window/Event.hpp>
#include "InputConfiguration.h"
#include "Logger.h"

std::unique_ptr<NS::Input> NS::Input::Instance_(nullptr);

NS::Input* NS::Input::Get()
{
	if (!Instance_)
	{
		Instance_ = std::unique_ptr<Input>(new Input());
	}

	return Instance_.get();
}

void NS::Input::Update(const std::optional<sf::Event>& Event)
{
	HandleAxes();
	HandleEvent(Event);
}

void NS::Input::BindOnKeyPressed(sf::Keyboard::Scancode Key, const InputActionBinding& Callback)
{
	int Index = static_cast<int>(Key);
	if (Index < 0 || Index >= sf::Keyboard::ScancodeCount)
	{
		NSLOG(ELogLevel::ERROR, "Invalid Scancode {} supplied for binding.", Index);
		return;
	}
	PressedCallbacks.at(Index) = Callback;
}

void NS::Input::UnBindOnKeyPressed(sf::Keyboard::Scancode Key)
{
	const int Index = static_cast<int>(Key);
	if (Index < 0 || Index >= sf::Keyboard::ScancodeCount)
	{
		NSLOG(ELogLevel::ERROR, "Invalid Scancode {} supplied for unbinding.", Index);
		return;
	}
	PressedCallbacks.at(Index) = nullptr;
}

void NS::Input::BindOnKeyReleased(sf::Keyboard::Scancode Key, const InputActionBinding& Callback)
{
	int Index = static_cast<int>(Key);
	if (Index < 0 || Index > sf::Keyboard::ScancodeCount)
	{
		NSLOG(ELogLevel::ERROR, "Invalid Scancode {} supplied for binding.", Index);
		return;
	}
	ReleasedCallbacks.at(Index) = Callback;
}

void NS::Input::UnBindOnKeyReleased(sf::Keyboard::Scancode Key)
{
	const int Index = static_cast<int>(Key);
	if (Index < 0 || Index >= sf::Keyboard::ScancodeCount)
	{
		NSLOG(ELogLevel::ERROR, "Invalid Scancode {} supplied for unbinding.", Index);
		return;
	}
	ReleasedCallbacks.at(Index) = nullptr;
}

void NS::Input::BindAxisHorizontal(const InputAxisBinding& Callback)
{
	HorizontalCallback = Callback;
}

void NS::Input::UnBindAxisHorizontal()
{
	HorizontalCallback = nullptr;
}

void NS::Input::BindAxisVertical(const InputAxisBinding& Callback)
{
	VerticalCallback = Callback;
}

void NS::Input::UnBindAxisVertical()
{
	VerticalCallback = nullptr;
}

void NS::Input::HandleAxes()
{
	float HorizontalValue = 0.0f;
	if (sf::Keyboard::isKeyPressed(NS::PositiveHorizontal))
	{
		HorizontalValue += 1.0f;
	}
	else if (sf::Keyboard::isKeyPressed(NS::NegativeHorizontal))
	{
		HorizontalValue -= 1.0f;
	}
	
	float VerticalValue = 0.0f;
	if (sf::Keyboard::isKeyPressed(NS::PositiveVertical))
	{
		VerticalValue += 1.0f;
	}
	else if (sf::Keyboard::isKeyPressed(NS::NegativeVertical))
	{
		VerticalValue -= 1.0f;
	}

	if (HorizontalCallback)
	{
		HorizontalCallback(HorizontalValue);
	}

	if (VerticalCallback)
	{
		VerticalCallback(VerticalValue);
	}
}

void NS::Input::HandleEvent(const std::optional<sf::Event>& Event)
{
	if (!Event)
	{
		return;
	}

	if (const auto* KeyPressedEvent = Event->getIf<sf::Event::KeyPressed>())
	{
		const sf::Keyboard::Scancode Scancode = KeyPressedEvent->scancode;
		const int Index = static_cast<int>(Scancode);
		if (Index < 0 || Index >= sf::Keyboard::ScancodeCount)
		{
			NSLOG(ELogLevel::ERROR, "Failed to handle input scancode {}.", Index);
			return;
		}
		auto& Callback = PressedCallbacks.at(Index);
		if (Callback)
		{
			Callback(Scancode);
		}
	}
	else if (const auto* KeyReleasedEvent = Event->getIf<sf::Event::KeyReleased>())
	{
		const sf::Keyboard::Scancode Scancode = KeyReleasedEvent->scancode;
		const int Index = static_cast<int>(Scancode);
		if (Index < 0 || Index >= sf::Keyboard::ScancodeCount)
		{
			NSLOG(ELogLevel::ERROR, "Failed to handle input scancode {}.", Index);
			return;
		}
		auto& Callback = ReleasedCallbacks.at(Index);
		if (Callback)
		{
			Callback(Scancode);
		}
	}
}