#include <iostream>
#include <fstream>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include "Character.h"

#define screenWidth 640
#define screenHeight 480

/*
Read in csv file describing the world map and writes it to 2D vector of ints.

Params:
    source - Path and name of csv file to load. 

Returns:
    2D vector describing the world map.
*/
std::vector<std::vector<int>> readWorldFile(const std::string& source)
{
    std::vector<std::vector<int>> returnMap;
    std::fstream stream;
    stream.open(source, std::ios::in);
    std::string line;
    while (getline(stream, line))
    {
        std::vector<int> values;
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());
        for (const auto& val : line)
        {
            values.push_back(std::stoi(std::string{ val }));
        }
        returnMap.push_back(values);
        std::cout << line.c_str() << std::endl;
    }
    return returnMap;
}

/*
Generates wall objects and their properties (color and location) and stores them in vector.

Params:
    worldMap - 2D vector describing the world dimensions and walls.

Returns:
    Vector of walls to populate world with.
*/
std::vector<sf::RectangleShape> generateWalls(std::vector<std::vector<int>>& worldMap)
{
    std::vector<sf::RectangleShape> walls;
    for (int i = 0; i < WORLD_BLOCK_HEIGHT; ++i)
    {
        for (int j = 0; j < WORLD_BLOCK_WIDTH; ++j)
        {
            if (worldMap[i][j])
            {
                sf::RectangleShape wall(sf::Vector2f(BLOCK_WIDTH, BLOCK_WIDTH));

                switch (worldMap[i][j])
                {
                case 1:
                    wall.setFillColor(sf::Color(175, 0, 0)); //red
                    break;
                case 2:
                    wall.setFillColor(sf::Color(0, 175, 0)); // green
                    break;
                case 3:
                    wall.setFillColor(sf::Color(0, 0, 175)); //blue
                    break;

                }
                wall.move(BLOCK_WIDTH * j, BLOCK_WIDTH * i);
                walls.push_back(wall);
            }
        }
    }
    return walls;
}

/*
Generates gridlines that are displayed in the world screen.

Returns:
    Vector of vertex arrays. Each vertex array describes a line with the first an second elements describing
    the beginning and end coordinates of the line.
*/
std::vector<std::array<sf::Vertex, 2>> generateGridLines()
{
    std::vector<std::array<sf::Vertex,2>> returnLines;
    for (float i = 0; i < WORLD_PIXEL_WIDTH; i += BLOCK_WIDTH)
    {
        std::array<sf::Vertex, 2> line = 
        {
            sf::Vertex(sf::Vector2f(i, 1.f)),
            sf::Vertex(sf::Vector2f(i, WORLD_PIXEL_HEIGHT))
        };
        returnLines.push_back(line);
    }
    for (float i = 0; i < WORLD_PIXEL_HEIGHT; i += BLOCK_WIDTH)
    {
        std::array<sf::Vertex, 2> line =
        {
            sf::Vertex(sf::Vector2f(1.f, i)),
            sf::Vertex(sf::Vector2f(WORLD_PIXEL_WIDTH, i))
        };
        returnLines.push_back(line);
    }
    return returnLines;
}

/*
Draws 3D window

Params:
    window3D - window to draw in. 
    character - character object that contains raycasting information to draw screen. 
*/
void draw3DWindow(sf::RenderWindow& window3D, Character& character)
{  
    //We draw a 1 pixel wide rectangle for each pixel column of the window.
    //The Rectangle's properties i.e, size and color, are determined by casting a ray from the character, 
    // through the camera plane at the corresponding angle, and recording how far the ray travels before hitting a wall. 
    //character.hits is a vector of structs describing each raycast. 
    for (int i = 0; i < character.getHits().size(); ++i)
    {
        //determine how tall the wall should be displayed based on it's distance. 
        double lineHeight = (1 / character.getHits()[i].distance) * screenHeight * BLOCK_WIDTH;

        //create rectangle 1 pixel wide
        sf::RectangleShape wall(sf::Vector2f(1.0f, lineHeight));

        //if vertical wall, shade wall to contrast with horizontal wall. 
        int colorAdjustment = character.getHits()[i].alignment == character.getHits()[i].vertical ? 0 : -25;

        switch (character.getHits()[i].color)
        {
        case 1:
            wall.setFillColor(sf::Color(175 + colorAdjustment, 0, 0));
            break;
        case 2:
            wall.setFillColor(sf::Color(0, 175 + colorAdjustment, 0));
            break;
        case 3:
            wall.setFillColor(sf::Color(0, 0, 175 + colorAdjustment));
            break;
        }
        //move wall so that its center is in the middle of the screen 
        wall.move(i, (screenHeight / 2) - (lineHeight / 2));
        window3D.draw(wall);
    }
}

/*
Draws 2D window

Params:
    window - window to draw in.
    gridlines - lines overlaid on world to more easily see measurments.
    walls - vector of wall objects to draw.
    character - object describing our character in the world. Contains position and raycasting information. 
    worldMap - 2D vector describing world layout. 
*/
void draw2DWindow(sf::RenderWindow& window, std::vector<std::array<sf::Vertex, 2>> gridLines, std::vector<sf::RectangleShape> walls, Character& character, std::vector<std::vector<int>>& worldMap)
{
    //draw gridlines
    for (const auto line : gridLines)
    {
        window.draw(&line[0], 2, sf::Lines);
    }
    //draw walls
    for (const auto& wall : walls)
    {
        window.draw(wall);
    }
    //draw character
    window.draw(character.getCharObject());
    
    //draw rays cast from character
    character.getRayCasts() = character.calcRays(character.getHits(), screenWidth, worldMap);
    for (const auto& cast : character.getRayCasts()
        )
    {
        sf::Vertex castingRay[] =
        {
            sf::Vertex(sf::Vector2f(character.getCenter().x, character.getCenter().y)),
            sf::Vertex(cast)
        };
        window.draw(castingRay, 2, sf::Lines);
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WORLD_PIXEL_WIDTH, WORLD_PIXEL_HEIGHT), "Map");
    sf::RenderWindow window3D(sf::VideoMode(screenWidth, screenHeight), "VectorMap");

    //read world description file 
    std::vector<std::vector<int>> worldMap = readWorldFile("res/map.csv");
    
    //generate walls 
    std::vector<sf::RectangleShape> walls = generateWalls(worldMap);
 
    //generate and draw gridlines
    std::vector<std::array<sf::Vertex, 2>> gridLines = generateGridLines();
    for (const auto& line : gridLines)
    {
        window.draw(&line[0], 2, sf::Lines);
    }

    //create character 
    Character character(16.f, -16, 0, 0, 16, sf::Color(100, 250, 50));

    // handle events
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                character.updateMovement(movementDirection::LEFT);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                character.updateMovement(movementDirection::RIGHT);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            {
                character.updateMovement(movementDirection::UP);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            {
                character.updateMovement(movementDirection::DOWN);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            {
                character.rotate(movementDirection::LEFT);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                character.rotate(movementDirection::RIGHT);
            }
        }

        window.clear();
        window3D.clear();

        draw2DWindow(window, gridLines, walls, character, worldMap);
        draw3DWindow(window3D, character);

        window.display();
        window3D.display();
    }
}