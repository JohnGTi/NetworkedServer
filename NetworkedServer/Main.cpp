#include <SFML/Graphics.hpp>

#include "ServerConsole.h"


int main() {

    printf("Server. For the purposes of a Fast-Action Networked Game Application.\n\n\n");

    sf::RenderWindow window(sf::VideoMode(200, 200), "Ground. SFML.");
    sf::CircleShape stapleShape(100.f);

    sf::Texture consoleStapleTexture;
    consoleStapleTexture.loadFromFile("Art/AngelDigitalQuality.png");

    stapleShape.setFillColor(sf::Color::White);
    stapleShape.setTexture(&consoleStapleTexture);


    // Through the server console, create a UDP socket and bind it to the server port.
    ServerConsole serverConsole;
    serverConsole.BindServer();

    // Receive incoming messages in executing infinite timeout selection.
    serverConsole.InitiateSelection();


    // Initialise objects for delta time.
    sf::Clock mainClock;
    float deltaTime;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Calculate delta time. How much time has passed since
        // it was last calculated (in seconds) and restart the clock.
        deltaTime = mainClock.restart().asSeconds();


        // Handle the servers outgoing backlog, send frequency permitting.
            //
        serverConsole.Update();


        window.clear();
        window.draw(stapleShape);
        window.display();
    }

    return 0;

}