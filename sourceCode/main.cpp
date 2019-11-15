#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>


const int winWidth = 1024;
const int winHeight = 768;

const int roadWidth = 1800;
const int segmentLength = 200;

unsigned  step = 0;     //camZ
float playerX = 0;      //camX
int camY = 1000;        //camH
unsigned playerZ = segmentLength*4;

int fov = 110;                              // field of view
float camDepth = 1/tan((fov/2)*3.14/180);   // distance from the camera to the screen
float frameTime = 0;
float animateTime = 0;
float accelerationTime = 0;

int speed = 0;
int maxSpeed = 0;
float speedPercentage;

sf::Texture bg;
sf::Sprite backGround;
sf::Image carTextures;
sf::Texture straight,
            left,
            leftLeft,
            right,
            rightRight;
sf::Sprite car;

void centerOrigin(sf::Sprite& sprite)
{
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width/2, bounds.height/2);
}

void drawSegment(sf::Color& c, sf::RenderWindow& window, int x, int y, int w, int x2, int y2, int w2)
{
    sf::ConvexShape segment(4);
    segment.setFillColor(c);
    segment.setPoint(0,sf::Vector2f(x-w,y));
    segment.setPoint(1,sf::Vector2f(x2-w2,y2));
    segment.setPoint(2,sf::Vector2f(x2+w2,y2));
    segment.setPoint(3,sf::Vector2f(x+w,y));
    window.draw(segment);
}

struct Line
{
    Line():
        curve{0},
        y{0},
        x{0},
        z{0}
    {
    }

    void project(float camX, int camY, unsigned& camZ, float& camDepth, const int& winWidth, const int& winHeight, const int& roadWidth)
    {
        scale = camDepth/(z-camZ);
        scrX = (1 + scale*(x - camX))*winWidth/2;
        scrY = (1 - scale*(y - camY))*winHeight/2;
        scrW = scale * roadWidth * winWidth/2;
    }

    float scale;
    float curve;
    float y;                //world coordinates
    int x, z;
    float scrX, scrY, scrW; //screen coordinates

};

void setCarPosition(sf::Sprite& car, int x, int y, int camY)
{
    float scale = camDepth/(playerZ);
    float Y = (1 - scale*(y-camY))*winHeight/2;
    car.setPosition(x,Y);
}

void turningAnimator(sf::Sprite& car, std::string direction, float& dTime, bool turning = true)
{
    if(turning)
    {
        if(animateTime < 200)
            animateTime+=dTime;
        if(direction == "left")
        {
            if(animateTime < 200) car.setTexture(left);
            else car.setTexture(leftLeft);

        }
        else if(direction == "right")
        {
            if(animateTime < 200) car.setTexture(right);
            else car.setTexture(rightRight);
        }
    }
    else
    {
        if(direction == "left")
            car.setTexture(straight);
        else if(direction == "right")
            car.setTexture(straight);
    }
}

void accelerate(int& speed, float& dTime)
{
    if(playerX > -1.2 && playerX < 1.2)
    {
        if(speed < maxSpeed)
            accelerationTime += dTime;
    }
    else
    {
        if(speed > 30)
            accelerationTime -= dTime*3;
        else if(speed < 30)
            accelerationTime += dTime;
        else
            speed = 30;
    }

    speed = 3*pow(accelerationTime/1000, 2);
    step += speed;
}

void deaccelerate(int& speed, float& dTime, bool breaking = false)
{
    if(playerX > -1.2 && playerX < 1.2)
    {
        if(speed > 0 && !breaking)
            accelerationTime -= dTime;
        else if( speed > 0 && breaking)
            accelerationTime -= dTime * 3;
    }
    else
    {
        if(speed > 0)
            accelerationTime-=dTime*4;
    }

    speed = 3*pow(accelerationTime/1000, 2);
    step += speed;
}

Line findSegment(std::vector<Line>& road, unsigned z)
{
    return road[(z/segmentLength)%road.size()];
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(winWidth, winHeight), "racingGame!");
    window.setFramerateLimit(60);

    //========================================================
    //
    //                 TEXTURES&SPRITES
    //
    //========================================================
    bg.loadFromFile("assets/bg.png");

    backGround.setTexture(bg);
    backGround.setPosition(0,0);

    carTextures.loadFromFile("assets/lotus.png");

    straight.loadFromImage(carTextures, sf::IntRect(129,0,62,31));
    left.loadFromImage(carTextures, sf::IntRect(66,0,62,31));
    leftLeft.loadFromImage(carTextures, sf::IntRect(0,0,62,31));
    right.loadFromImage(carTextures, sf::IntRect(191,0,62,31));
    rightRight.loadFromImage(carTextures, sf::IntRect(254,0,62,31));

    car.setTexture(straight);
    car.setScale(sf::Vector2f(3,3));
    centerOrigin(car);
    setCarPosition(car, winWidth/2, segmentLength, camY);

    //========================================================
    //
    //                 ROAD_CONTAINER
    //
    //========================================================
    std::vector<Line> road;
    for(int i = 0; i < 2000; i++)
    {
        Line line;
        //  ROAD_GEOMETRY
        //  [+] right    :   [-] left
        if(i > 50 && i < 300) line.curve = 2;
        if(i > 300 && i < 700) line.curve = -2.5;
        if(i > 700 && i < 900) line.curve = 3.3;
        if(i > 1100 && i < 1500) line.curve = -1.5;
        if(i > 1550 && i < 1700) line.curve = -3.5;
        if(i > 1700 && i < 1850) line.curve = 1;

        //  [+] downwards    :   [-] upwards
        if(i > 0 && i < 100) line.y = 0.5;
        if(i > 100 && i < 250) line.y = -0.8;
        if(i > 320 && i < 600) line.y = 0.6;
        if(i > 700 && i < 800) line.y = -1;
        if(i > 800 && i < 1000) line.y = 0.5;
        if(i > 1500 && i < 1800) line.y = -0.5;
        if(i > 1800 && i < 1850) line.y = 0.5;
        if(i > 1850 && i < 1900) line.y = -0.7;
        if(i > 1900 && i < 1970) line.y = 0.7;

        road.push_back(line);
    }

    sf::Clock clock;
    //========================================================
    //
    //                 GAME_LOOP
    //
    //========================================================
    while (window.isOpen())
    {
        float frameStartTime = clock.getElapsedTime().asMilliseconds();
        sf::Event e;

        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed  || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            accelerate(speed,frameTime);
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            deaccelerate(speed,frameTime,true);
        else
            deaccelerate(speed,frameTime);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            if(playerX > -1.35)
            {
                //playerX -= 0.015*2*(-0.02*pow(speedPercentage,2)+2.835*speedPercentage)*0.01;
                playerX -= 0.04*(-0.0169*pow(speedPercentage,2)+2.6*speedPercentage)*0.01;
                //playerX -= 0.015*2/*speedPercentage*0.01*/;
            }
            turningAnimator(car,"left", frameTime);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            if(playerX < 1.35)
            {
                //playerX += 0.015*2*(-0.02*pow(speedPercentage,2)+2.835*speedPercentage)*0.01;
                playerX += 0.04*(-0.0169*pow(speedPercentage,2)+2.6*speedPercentage)*0.01;
                //playerX += 0.015*2/*speedPercentage*0.01*/;
            }
            turningAnimator(car,"right", frameTime);
        }

        if(e.type == sf::Event::KeyReleased)
        {
            if(e.key.code == sf::Keyboard::Left || e.key.code == sf::Keyboard::Right)
            {
                if(e.key.code == sf::Keyboard::Left) turningAnimator(car,"left", frameTime, false);
                if(e.key.code == sf::Keyboard::Right) turningAnimator(car,"right", frameTime, false);
                animateTime = 0;
            }
        }

        if(step > road.size()*segmentLength) step = 0;

        unsigned pos = step/segmentLength;

        //int camH = camY + road[pos].y;
        int maxY = winHeight;
        float dx = 0;
        float x = 0;

        int camH = camY;
        float dy = 0;
        float y = 0;
        maxSpeed = (segmentLength / 15 * 10) * 2;

        //speedPercentage = (speed*100/maxSpeed)*0.01;
        speedPercentage = (speed*100/maxSpeed);

        if(playerX > -1.35 && playerX < 1.35)
            playerX = playerX - (0.05 * (speedPercentage * 0.01) * (findSegment(road,step+playerZ).curve) * 0.25);

        //std::cout<<playerX<<" "<<(-0.0169*speedPercentage*speedPercentage+2.6*speedPercentage)*0.01<<std::endl;
        //std::cout<<accelerationTime<<" "<<speed<<std::endl;

    //========================================================
    //
    //                 WINDOW_DRAW
    //
    //========================================================
        window.clear();
        window.draw(backGround);
        for(unsigned i = pos + 1; i <= pos + 300; i++)
        {
            unsigned n = i;
            if(i >= road.size()) n = i - road.size();

            if(i == 1)
            {
                road[0].z = (i*segmentLength+segmentLength);
                road[0].project(playerX * roadWidth - x, camH + y, step, camDepth, winWidth, winHeight, roadWidth);
            }

            road[n].z = (i * segmentLength + segmentLength);    //world z coordinate for segment
            road[n].project(playerX * roadWidth - x, camH + y, step, camDepth, winWidth, winHeight, roadWidth);

            Line l = road[n];       //current segment

            x += dx;
            dx += l.curve;

            y += dy;
            dy += l.y;

            if(l.scrY >= maxY) continue;
            maxY = l.scrY;

            sf::Color grass;
            sf::Color tarmac;
            sf::Color curbs;

            (n/3)%2 == 0?       grass = sf::Color(0,154,0)      : grass = sf::Color(0,197,0);
            (n/3)%2 == 0?       tarmac = sf::Color(121,123,121) : tarmac = sf::Color(142, 140, 142);
            (n/3)%2 == 0?       curbs = sf::Color::White        : curbs = sf::Color::Red;
            if(n == 4)          tarmac = sf::Color::White;

            Line p = road[n-1];     //previous segment

            if(n == 0 )
                p = road.back();

            drawSegment(grass, window, 0, p.scrY, winWidth,0 , l.scrY, winWidth);
            drawSegment(curbs, window, p.scrX, p.scrY, p.scrW*1.1, l.scrX, l.scrY, l.scrW*1.1);
            drawSegment(tarmac, window, p.scrX, p.scrY, p.scrW, l.scrX, l.scrY, l.scrW);
        }

        window.draw(car);
        window.display();

        frameTime = clock.getElapsedTime().asMilliseconds() - frameStartTime;
    }

    return EXIT_SUCCESS;
}
