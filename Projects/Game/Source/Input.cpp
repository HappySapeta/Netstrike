#include "Input.h"

#include <optional>
#include <SFML/Window/Event.hpp>
#include "InputConfiguration.h"

std::unique_ptr<NS::Input> NS::Input::Instance_(nullptr);

void NS::Input::Update(const std::optional<sf::Event>& Event)
{
	HandleAxes();
	
	if (!Event)
	{
		return;
	}
}

NS::Input* NS::Input::Get()
{
	if (!Instance_)
	{
		Instance_ = std::make_unique<Input>();
	}

	return Instance_.get();
}

void NS::Input::HandleAxes()
{
	float HorizontalValue = 0.0f;
	if (sf::Keyboard::isKeyPressed(NS::PositiveHorizontal))
	{
		HorizontalValue += 1.0f;
	}
	
	if (sf::Keyboard::isKeyPressed(NS::NegativeHorizontal))
	{
		HorizontalValue -= 1.0f;
	}
	
	float VerticalValue = 0.0f;
	if (sf::Keyboard::isKeyPressed(NS::PositiveVertical))
	{
		VerticalValue += 1.0f;
	}
	
	if (sf::Keyboard::isKeyPressed(NS::NegativeVertical))
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

void NS::Input::BindAxisHorizontal(const InputAction& Callback)
{
	HorizontalCallback = Callback;
}

void NS::Input::UnBindAxisHorizontal()
{
	HorizontalCallback = nullptr;
}

void NS::Input::BindAxisVertical(const InputAction& Callback)
{
	VerticalCallback = Callback;
}

void NS::Input::UnBindAxisVertical()
{
	VerticalCallback = nullptr;
}
