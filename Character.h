#pragma once

#include <array>
#include <SFML/OpenGL.hpp>

static constexpr int WORLD_PIXEL_WIDTH = 1024;
static constexpr int WORLD_PIXEL_HEIGHT = 512;
static constexpr float MOVEMENT_SPEED{ 2.0f };
static constexpr double BLOCK_WIDTH{ 32.0f };
static constexpr int WORLD_BLOCK_WIDTH = 1024 / BLOCK_WIDTH;
static constexpr const int WORLD_BLOCK_HEIGHT = 512 / BLOCK_WIDTH;

enum movementDirection
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Character
{

private:

    //Contains details about a ray hitting a wall. Includes distance ray travelled, 
    //wall color, and whether the wall is horizontal. 
    struct hitDetails
    {
        enum Alignment
        {
            horizontal,
            vertical,
            unknown
        };

        double distance{ 0.0 };
        int color{ 0 };
        Alignment alignment{ unknown };
    };


    //character is a circle, so radius of the character. 
    //Is also the distance from the character center to the camera plane. 
    float characterRadius;
    //Character's direction or 'where they are looking' is described by a ray extending from the center of the character. 
    sf::Vertex directionRay[2];

    //camera plane is perpendicular to the end of the direction ray. It is used in determing the field of view from the character perspective.
    //It is used to determine the angle a ray should be cast from the center of the character through the camera plane.
    //For example assume the cameraPlane is 100 units long and the 3D display is 100 pixels wide. The first ray's is cast from the center of the character 
    //through unit 1 of the camera plane and into the world to draw the first pixel column. The second ray is cast through unit 2 of the camera plane, etc.   
    //The longer the camera plane, the greater the FOV. 
    sf::Vertex cameraPlane[2];

    //sfml representation of character 
    sf::CircleShape charObject;

    //direction coordinates relative to character center 
    double dirX;
    double dirY;

    //cameraPlane center relative to character center 
    double cameraPlaneX;
    double cameraPlaneY;

    //center of character
    sf::Vector2f center;

    //vector of structs describing results of casting rays. 
    std::vector<hitDetails> hits;

    //vector of raycast sfml objects used to display where rays are cast in scene. 
    std::vector<sf::Vertex> rayCasts;

public:

    ~Character() = default;

    Character(float size, double dirX, double dirY, double cameraPlaneX, double cameraPlaneY, sf::Color color) :
        characterRadius(size), dirX(dirX), dirY(dirY), cameraPlaneX(cameraPlaneX), cameraPlaneY(cameraPlaneY)
    {
        charObject = sf::CircleShape(16.0f);
        center = getCharacterCenter();

        auto directionRayEnd = sf::Vector2f(getCharacterCenter().x + dirX, getCharacterCenter().y + dirY);

        directionRay[0] = sf::Vertex(getCharacterCenter());
        directionRay[1] = sf::Vertex(directionRayEnd);
        cameraPlane[0] = sf::Vector2f(directionRayEnd.x - cameraPlaneX, directionRayEnd.y - cameraPlaneY);
        cameraPlane[1] = sf::Vertex(sf::Vector2f(directionRayEnd.x + cameraPlaneX, directionRayEnd.y + cameraPlaneY));

        charObject.setFillColor(color);


    }


    /*
    Move Character in 2D space. 

    Params:
        xDistance - distance to move along X axis.
        yDistance - distance to move along X axis.
    */
    void move(double xDistance, double yDistance)
    {
        charObject.move(xDistance, yDistance);
    }

    /*
    Get center coordinates of character object.

    Returns:
        Vector2f of X and Y coordinates. 
    */
    sf::Vector2f getCharacterCenter()
    {
        return sf::Vector2f(charObject.getPosition().x + characterRadius, charObject.getPosition().y + characterRadius);
    }
    
    /*
    Rotates character in 2D space. This requires rotating the character, the direction vector, and camera plane. 
    
    Params: 
        direction - enum of either LEFT or RIGHT indicating how to rotate values. 
    */
    void rotate(movementDirection dir)
    {
        double oldDirX = dirX;
        double oldDirY = dirY;

        double spinDir = (dir == movementDirection::LEFT) ? -1.0 : 1.0;
        dirX = dirX * cos(spinDir * 0.01) - dirY * sin(spinDir * 0.01);
        dirY = oldDirX * sin(spinDir * 0.01) + dirY * cos(spinDir * 0.01);

        auto directionRayEnd = getCharacterCenter();
        directionRayEnd.x += dirX;
        directionRayEnd.y += dirY;
        directionRay[0] = sf::Vertex(getCharacterCenter());
        directionRay[1] = sf::Vertex(directionRayEnd);

        double oldPlaneX = cameraPlaneX;
        double oldPlaneY = cameraPlaneY;

        cameraPlaneX = cameraPlaneX * cos(spinDir * 0.01) - cameraPlaneY * sin(spinDir * 0.01);
        cameraPlaneY = oldPlaneX * sin(spinDir * 0.01) + cameraPlaneY * cos(spinDir * 0.01);

        auto startPlanePosition = directionRayEnd;
        auto endPlanePosition = startPlanePosition;
        endPlanePosition.x += cameraPlaneX;
        endPlanePosition.y += cameraPlaneY;
        cameraPlane[0] = sf::Vertex(startPlanePosition);
        cameraPlane[1] = sf::Vertex(endPlanePosition);
    };

    /*
    Moves character (up, down, left right) in 2D space and updates camera plane and direction vector.

    Params:
        direction - enum of either UP, DOWN, LEFT or RIGHT indicating how to move values.
    */
    void updateMovement(movementDirection direction)
    {
        float xAdjustment = 0.0f;
        float yAdjustment = 0.0f;

        switch (direction)
        {
        case LEFT:
            xAdjustment -= MOVEMENT_SPEED;
            move(-MOVEMENT_SPEED, 0.f);
            center.x -= MOVEMENT_SPEED;
            break;
        case RIGHT:
            xAdjustment += MOVEMENT_SPEED;
            center.x += MOVEMENT_SPEED;
            move(MOVEMENT_SPEED, 0.f);
            break;
        case UP:
            yAdjustment -= MOVEMENT_SPEED;
            center.y -= MOVEMENT_SPEED;
            move(0.f, -MOVEMENT_SPEED);
            break;
        case DOWN:
            yAdjustment += MOVEMENT_SPEED;
            center.y += MOVEMENT_SPEED;
            move(0.f, MOVEMENT_SPEED);
            break;
        }
        cameraPlane[0].position = sf::Vector2f(cameraPlane[0].position.x + xAdjustment, cameraPlane[0].position.y + yAdjustment);
        cameraPlane[1].position = sf::Vector2f(cameraPlane[1].position.x + xAdjustment, cameraPlane[1].position.y + yAdjustment);
        directionRay[0].position = sf::Vector2f(directionRay[0].position.x + xAdjustment, directionRay[0].position.y + yAdjustment);
        directionRay[1].position = sf::Vector2f(directionRay[1].position.x + xAdjustment, directionRay[1].position.y + yAdjustment);
    }

    /*
    Determine distance along the ray to reach the next closest gridline for a given axis (X or Y).
    This function operates on one axis. So to get the closest X or Y gridline you will have to run this function twice: 
    once for the X axis and once for the Y axis and then compare to see which gridline is closer. 

    Params:
        vertex - current coordinate along axis we are measuring from. 
        rayDir - Direction of ray, we really only care if going in negative (up and left) or positive direction (down and right). 
        deltaDistance - distance between two parallel gridlines when traversing the ray. 
     */
    double calcNewDistance(int vertex, double rayDir, double deltaDist)
    {
        double distance = int(vertex) % (int)BLOCK_WIDTH;
        if (rayDir < 0 && distance == 0)
        {
            distance = BLOCK_WIDTH;
        }
        else if (rayDir >= 0)
        {
            distance = BLOCK_WIDTH - distance;
            if (distance == 0)
            {
                distance = BLOCK_WIDTH;
            }
        }
        return (distance / BLOCK_WIDTH) * deltaDist;
    };


    /*
    Check if the current location intersects with a wall and if so record the intersection details. 

    Params:
        xIndex - X coordinate we are testing.
        yIndex - Y coordinate we are testing. 
        hitDetail - if intersection occurred, object that will store intersection details.
        worldMap - 2D vector describing the environment 
    Returns:
        True if intersection occurred, otherwise false. 
     */
    bool checkForHit(double xIndex, double yIndex, hitDetails& hitDetail, std::vector<std::vector<int>>& worldMap)
    {
        bool hit{false};

        //double intersection
        if (std::floor(xIndex) == xIndex && std::floor(yIndex) == yIndex)
        {
            int x = xIndex;
            int y = yIndex;

            if (worldMap[y][x] != 0)
            {
                hit = true;
                hitDetail.color = worldMap[y][x];
            }
            else if (worldMap[y - 1][x - 1] != 0)
            {
                hit = true;
                hitDetail.color = worldMap[y - 1][x - 1];

            }
            else if (worldMap[y][x - 1] != 0)
            {
                hit = true;
                hitDetail.color = worldMap[y][x - 1];
            }
            else if (worldMap[y - 1][x] != 0)
            {
                hit = true;
                hitDetail.color = worldMap[y - 1][x];
            }
        }
        else if (std::floor(xIndex) == xIndex)
        {
            int x = xIndex;
            int y = std::floor(yIndex);
            if (worldMap[y][x] != 0 || worldMap[y][x - 1] != 0)
            {
                hitDetail.alignment = hitDetail.vertical;
                hit = true;
                if (worldMap[y][x] != 0)
                {
                    hitDetail.color = worldMap[y][x];
                }
                else
                {
                    hitDetail.color = worldMap[y][x - 1];
                }
            }
        }
        else if (std::floor(yIndex) == yIndex)
        {
            int y = yIndex;
            int x = std::floor(xIndex);
            if (worldMap[y][x] != 0 || worldMap[y - 1][x] != 0)
            {
                hitDetail.alignment = hitDetail.horizontal;
                hit = true;
                hitDetail.color = worldMap[y][x] ? worldMap[y][x] : worldMap[y - 1][x];
            }
        }
        return hit;
    }

    /*
    Calculate ray distances for each screen pixel and color of surface being hit.
    */
    std::vector<sf::Vertex> calcRays( std::vector<hitDetails>& hits, int screenWidth, std::vector<std::vector<int>>& worldMap)
    {
        hits.clear();
        std::vector<sf::Vertex> rayCasts;

        //Each column of pixels in the screen gets a calculation. calculate the size of wall seen for that column and its color. Creates illusion of 3D.  
        for (double i = 0; i <= screenWidth; ++i)
        {
            //map position that will change over time 
            int mapX = int(center.x);
            int mapY = int(center.y);

            //determine where in the camera plane our ray for the pixel column intersects
            double cameraX = 2 * i / double(screenWidth) - 1;
            double rayDirX = dirX + cameraPlaneX * cameraX;
            double rayDirY = dirY + cameraPlaneY * cameraX;

            //determine deltaDistX and deltaDistX. The deltas can be describe as hypoteneus created by X and Y direction rays. 
            double hypoLength = sqrt(rayDirX * rayDirX + rayDirY * rayDirY);
            double xMultiplier = 32.0f / std::abs(rayDirX);
            double yMultiplier = 32.0f / std::abs(rayDirY);
            double deltaDistX = (rayDirX == 0) ? 1e30 : xMultiplier * hypoLength;
            double deltaDistY = (rayDirY == 0) ? 1e30 : yMultiplier * hypoLength;

            //apply + and - valuse to deltas. Up and Left are (-). Right and Down are (+).
            deltaDistX = rayDirX < 0 ? deltaDistX * -1 : deltaDistX;
            deltaDistY = rayDirY < 0 ? deltaDistY * -1 : deltaDistY;

            //calculate initial distances to sides. TODO: make readable. 
            double sideDistX = rayDirX < 0 ? ((int(center.x) % 32) / 32.0f) * deltaDistX :
                ((32.0f - (int(center.x) % 32)) / 32.0f) * deltaDistX;
            double sideDistY = rayDirY < 0 ? ((int(center.y) % 32) / 32.0f) * deltaDistY :
                ((32.0f - (int(center.y) % 32)) / 32.0f) * deltaDistY;

            bool hit{ false };

            hitDetails temp;
            while (!hit)
            {
                //determine which is closer along the ray's path: intersecting a Y or X grid line. 
                double adjustmentDistance = (std::abs(sideDistX) <= std::abs(sideDistY)) ? sideDistX : sideDistY;
                //determine how far along ray we must travel to reach the next intersection. 
                double adjustment = (adjustmentDistance / deltaDistX) * 32.0;

                double signOperator = deltaDistX > 0 ? 1 : -1;
                adjustment = std::abs(adjustment) * signOperator;
                mapX += std::round(adjustment);

                signOperator = deltaDistY > 0 ? 1 : -1;
                adjustment = std::abs((adjustmentDistance / deltaDistY) * 32.f) * signOperator;
                mapY += std::round(adjustment);

                //now calc the new distances to X and Y grids after moving
                sideDistX = calcNewDistance(mapX, rayDirX, deltaDistX);
                sideDistY = calcNewDistance(mapY, rayDirY, deltaDistY);

                //check against world map to see what block we are up against 
                double xIndex = mapX / 32.0f;
                double yIndex = mapY / 32.0f;
                hit = checkForHit(xIndex, yIndex, temp, worldMap);
                if (i && hit && temp.alignment == temp.unknown)
                {
                    temp.alignment = hits[i-1].alignment;
                }
            }
            //calc distance for each ray 
            double startX = center.x;
            double startY = center.y;
            double endX = mapX;
            double endY = mapY;
            double distance = std::sqrt((startX - endX) * (startX - endX) + (startY - endY) * (startY - endY));
            temp.distance = distance;
            rayCasts.push_back(sf::Vector2f(mapX, mapY));
            hits.push_back(temp);
        }
        return rayCasts;
    }

    auto& getHits() {
        return hits;
    }

    auto& getCenter()
    {
        return center;
    }

    auto& getCharObject()
    {
        return charObject;
    }

    auto& getRayCasts()
    {
        return rayCasts;
    }
};