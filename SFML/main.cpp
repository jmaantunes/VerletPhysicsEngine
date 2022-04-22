#include <SFML/Graphics.hpp>

#include <iostream>
#include <cmath>
#include <vector>





//global

const int width = 1200;
const int height = 1000;

const float FPS = 60.0f;


const uint32_t subSteps = 4;
const float dt = 1/FPS;
const float subDt = dt/(float)subSteps;

const int objTotal = 800;

const sf::Vector2f constraintCenter = {width/2, height/2};
const float constraintRadius = 450.0f;



void loopColor(int &count, int &r, int &g, int &b){

    int speed = 4;

    if(count >= 0 && count <= 255){
        g+=speed;
    } else if(count > 255 && count <= 255*2){
        r-=speed;
        
    } else if(count > 255*2 && count <= 255*3){
        b+=speed;

    } else if(count > 255*3 && count <= 255*4){
        g-=speed;
        
    } else if(count > 255*4 && count <= 255*5){
        r+=speed;
        
    } else if(count > 255*5){
        count = 0;
        r=255;
        g=0;
        b=0;
    }

    count+=speed;
}




float getVectorLength(sf::Vector2f vec){
    return sqrt(vec.x*vec.x + vec.y*vec.y);
}

float getRandomFloat(float min, float max){
    float random = ((float) rand()) / (float) RAND_MAX;

    float range = max - min;  
    return (random*range) + min;
}


struct VerletObject {
    sf::CircleShape shape;
    float radius;
    sf::Vector2f currentPos;
    sf::Vector2f oldPos;
    sf::Vector2f acceleration;

    VerletObject(){}

    VerletObject(float x, float y){

        const int maxRadius = 12;
        const int minRadius = 18;

        radius = getRandomFloat(minRadius,maxRadius);
        //radius = 10;
        //std::cout << "rad: " << radius << std::endl;

        shape = sf::CircleShape(radius);

        currentPos = sf::Vector2f(x,y);
        oldPos = currentPos;
        oldPos = sf::Vector2f(x,y-maxRadius/subSteps*3);

        acceleration = {};

        shape.setPosition(0,0);
        shape.setOrigin(radius,radius);
        shape.setPosition(x,y);

    }

    void updatePosition(float dt){
        const sf::Vector2f velocity = currentPos - oldPos;

        // Save current pos
        oldPos = currentPos;

        // Perform Verlet integration
        currentPos = currentPos + velocity + acceleration * dt * dt;
        shape.setPosition(currentPos);

        // Reset acceleration
        acceleration = {};
    }

    void accelerate(sf::Vector2f acc){
        acceleration += acc;
    }

    void setColor(int r, int g, int b){
        shape.setFillColor(sf::Color(r,g,b));
    }
};



struct ColorsCache {
    
    sf::Color colorArray[objTotal];

    ColorsCache(){}
    ColorsCache(std::vector<VerletObject> objList){

        for (int i = 0; i < objTotal; ++i){
            VerletObject& objA = objList[i];
            colorArray[i] = objA.shape.getFillColor();

        }

    }

    
};

ColorsCache cache;

struct Solver
{
    const sf::Vector2f gravity = {0.0f, 4000.0f};

    std::vector<VerletObject> objList;

    sf::CircleShape constraintShape;
    
    

    Solver(){
        constraintShape = sf::CircleShape(constraintRadius);
        constraintShape.setFillColor(sf::Color::Black);

        constraintShape.setPosition(0,0);
        constraintShape.setOrigin(constraintRadius,constraintRadius);
        constraintShape.setPosition(constraintCenter);

        resetRandSeed();
        
    }

    void resetRandSeed(){
        srand(1); //deterministic simulation
    }

    void addObject(float x, float y, int r, int g, int b){
        VerletObject newObj = VerletObject(x, y);
        newObj.setColor(r,g,b);
        objList.push_back(newObj);
    }

    void update(){
        for (uint32_t i = 0; i < subSteps; i++)
        {
            applyGravity();
            applyConstraint();
            solveCollisions();
            updatePositions(subDt);
        }
    }

    void updatePositions(float dt){
        for(VerletObject &obj : objList){
            obj.updatePosition(dt);
        }
    }

    void applyGravity(){
        for(VerletObject &obj : objList){
            obj.accelerate(gravity);
        }
    }

    void applyConstraint(){
        for(VerletObject &obj : objList){
            
            const sf::Vector2f toObj = obj.currentPos - constraintCenter;

            const float distToObj = getVectorLength(toObj);

            const float maxDist = constraintRadius - obj.radius;
            if(distToObj > maxDist){
                const sf::Vector2f normalized = toObj / distToObj;
                obj.currentPos = constraintCenter + normalized * maxDist;
            }

        }
    }

    void solveCollisions(){

        const int objCount = objList.size();

        for (int i = 0; i < objCount; ++i){
            VerletObject& objA = objList[i];
            for (int k = i+1; k < objCount; ++k){
                VerletObject& objB = objList[k];

                
                const sf::Vector2f collisionAxis = objA.currentPos - objB.currentPos;
                const float dist = getVectorLength(collisionAxis);

                const float minDist = objA.radius + objB.radius;

                if(dist < minDist){
                    sf::Vector2f n = collisionAxis / dist;
                    const float delta = minDist - dist;

                    //float proportionA = radiusA/minDist; //not needed

                    objA.currentPos += 0.5f * delta * n;
                    objB.currentPos -= 0.5f * delta * n;
                }
            }
        }
        
    }

    void transferColorsFromImage(sf::Image logo){
        for(VerletObject &obj : objList){
            const sf::Vector2f positionInImage = obj.currentPos - (constraintCenter - sf::Vector2f(constraintRadius, constraintRadius));

            sf::Color pixelColor = logo.getPixel(positionInImage.x, positionInImage.y);

            obj.setColor(pixelColor.r, pixelColor.g, pixelColor.b);

        }
        std::cout << "Loaded colors from image\n";


    }

    void clearObjects(){
        objList.clear();
        resetRandSeed();
        std::cout << "Objects cleared, seed reset\n";
    }

    void saveColors(){

        cache = ColorsCache(objList);
        std::cout << "Colors saved!\n";

    }

    void loadColors(){

        //load colors
        const int objCount = objList.size();
        for (int i = 0; i < objCount; ++i){
            VerletObject& obj = objList[i];

            sf::Color cachedColor = cache.colorArray[i];
            
            obj.setColor(cachedColor.r, cachedColor.g, cachedColor.b);

        }

        std::cout << "Colors loaded!\n";
        

    }


};




int main(){
    
    bool usingCachedColors = false;

    sf::Image logo;
    if (!logo.loadFromFile("logo.png")){
        std::cout << "image load error\n";
    }
    


    sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

    sf::RenderWindow window(sf::VideoMode(width, height), "SFML Physics Engine", sf::Style::Default, settings);
    window.setFramerateLimit(FPS);
    

    Solver mySolver = Solver();

    int count=0;
    int r=255;
    int g=0;
    int b=0;

    int objCount = objTotal;
    
    while (window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){
            
            if (event.type == sf::Event::Closed){
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed){
                switch(event.key.code){
                    case sf::Mouse::Left: {

                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        int x = mousePos.x;
                        int y = mousePos.y;

                        //std::cout << "left press on (" << x << "," << y << ")\n";
                    
                        //mySolver.addObject(x,y, r,g,b);

                    } break;
                }

            } else if (event.type == sf::Event::KeyPressed){
                switch(event.key.code){
                    
                    case sf::Keyboard::S: {
                        mySolver.saveColors();
                    } break;

                    case sf::Keyboard::Q: {
                        mySolver.transferColorsFromImage(logo);
                    } break;

                    
                    case sf::Keyboard::A: {
                        std::cout << "Spawning objects...\n";
                        objCount = 0;
                        
                    } break;

                    case sf::Keyboard::C: {
                        mySolver.clearObjects();

                        usingCachedColors = true;
                        
                    } break;


                }
            }
        }

        loopColor(count,r,g,b);
        

        if(objCount < objTotal){
            mySolver.addObject(width/3, height/7, r,g,b);

            if(usingCachedColors){
                sf::Color cachedColor = cache.colorArray[objCount];
                mySolver.objList[objCount].setColor(cachedColor.r, cachedColor.g, cachedColor.b);
            }

            objCount++;
        }
        
        
        mySolver.update();
        window.clear(sf::Color(158,158,158));
        window.draw(mySolver.constraintShape);

        for (VerletObject obj : mySolver.objList){
            window.draw(obj.shape);
        }

        
        window.display();
    }

    return 0;
}