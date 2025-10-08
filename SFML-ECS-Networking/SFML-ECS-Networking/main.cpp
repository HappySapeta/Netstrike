/*	CMP425/501 Assessment Project Template.
* 
*	This is an template project that renders an empty SFML window.
*	Use this as a starting point of your final assessment.
*	There are two build configurations -- client and server. 
*	You can include/exclude source files for each configuration, 
*	if you need to build different versions of your game.
*/

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <sstream>
#include <iomanip>
#include "utils.h"

int main() {
	Utils::printMsg("Game startup...");

	// Prepare window.
	sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "CMP425/CMP501 - Assessment Template");
	window.setFramerateLimit(60);	//Request 60 frames per second
	Utils::printMsg("Window ready...");

	// Clock for timing the 'dt' value.
	sf::Clock clock;

	// Custom clear colour for the window (light blue).
	sf::Color clear_colour(135, 206, 250);

	while (window.isOpen()) {
		// Calculate dt.
		float dt = clock.restart().asSeconds();

		// Handle window events (e.g. key press).
		while (const std::optional event = window.pollEvent())	{
			// This event is triggered when the "x" button is pressed to close the window.
			if (event->is<sf::Event::Closed>()) {
				Utils::printMsg("Window closed...", MessageType::warning);
				window.close();
			}
			// This event checks key press and closes the window on "ESC".
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
					Utils::printMsg("ESC pressed, closing window...", MessageType::warning);
					window.close();
				}
			}
		}
		window.clear(clear_colour);

		// Render you game objects here

		window.display();		
	}

	return 0;
}

