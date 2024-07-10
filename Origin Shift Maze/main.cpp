#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <iostream>

#include "EasyRandom.h"
#include "numcpp.h"
#include "NeuralNetwork.h"
using namespace std;

enum class Directions { Right, Down, Left, Up, None };
static sf::Vector2i dirToCoords(Directions shift)
{
    switch (shift)
    {
    case Directions::Right:
        return { 1, 0 };
        break;
    case Directions::Down:
        return { 0, 1 };
        break;
    case Directions::Left:
        return { -1, 0 };
        break;
    case Directions::Up:
        return { 0, -1 };
        break;
    case Directions::None:
        return { 0, 0 };
        break;
    }
}

class Maze 
{   
    sf::Vector2f clamp(sf::Vector2f vec)
    {
        return { max(1.0f, vec.x), max(1.0f, vec.y) };
    }

public:
    sf::Vector2i windowSize;
    int width = 0, height = 0;

    sf::Vector2i offset;
    float baseSize = 0;

    vector<vector<Directions>> field;
    vector<sf::RectangleShape> path;
    vector<sf::RectangleShape> walls;
    
    sf::Vector2i origin;
    sf::RectangleShape originObj;
    sf::Vector2i finishPos;
    sf::RectangleShape finishObj;

    EasyRandom gen;

    Maze(int w, int h, sf::RenderWindow& window, sf::Vector2i finPos = { -1, -1 })
    {
        setup(w, h, window, finPos);
    }

    void setup(int w, int h, sf::RenderWindow& window, sf::Vector2i finPos = { -1, -1 })
    {
        width = w;
        height = h;
        windowSize = sf::Vector2i(window.getSize());

        field.clear();
        field.resize(h, vector<Directions>(w));
        path.clear();
        path.resize(w * h);
        walls.clear();
        walls.resize(2 * w * h + w + h);
        gen.set({ 0, 3 });

        if (finPos == sf::Vector2i(-1, -1)) finishPos = sf::Vector2i(width - 1, height - 1);

        origin = sf::Vector2i(w - 1, h - 1);
        //Лабиринт занимает не более 9/10 экрана по каждому из измерений, выбираем размер клетки
        baseSize = min(windowSize.x * 9.0 / 10 / w, windowSize.y * 9.0 / 10 / h);
        offset = sf::Vector2i((windowSize.x - w * baseSize) / 2, (windowSize.y - h * baseSize) / 2);

        originObj.setSize(clamp(sf::Vector2f(baseSize / 7.5, baseSize / 7.5)));
        originObj.setOrigin(sf::Vector2f(baseSize / 15.0, baseSize / 15.0));
        originObj.setPosition(sf::Vector2f(offset) + sf::Vector2f(baseSize / 2.0, baseSize / 2.0) + baseSize * sf::Vector2f(origin));
        originObj.setFillColor(sf::Color::Red);

        finishObj.setSize(clamp(sf::Vector2f(baseSize / 7.5, baseSize / 7.5)));
        finishObj.setOrigin(sf::Vector2f(baseSize / 15.0, baseSize / 15.0));
        finishObj.setPosition(sf::Vector2f(offset) + sf::Vector2f(baseSize / 2.0, baseSize / 2.0) + baseSize * sf::Vector2f(finishPos));
        finishObj.setFillColor(sf::Color::Magenta);

        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                //field[i][j] = Directions(gen.get());
                field[i][j] = Directions::Right;
                if (j == width - 1) field[i][j] = Directions::Down;
                if (i == height - 1 && j == width - 1) field[i][j] = Directions::None;
            }
        }
        for (int i = 0; i < w * h * 30; i++) {
            originShift();
        }
        buildPath();
        buildWalls();
    }

    void originShift()
    {
        Directions shift = Directions(gen.get());
        sf::Vector2i newOrigin = origin + dirToCoords(shift);
        sf::IntRect box(0, 0, width, height);
        while (!box.contains(newOrigin))
        {
            shift = Directions(gen.get());
            newOrigin = origin + dirToCoords(shift);
        }
                
        field[origin.y][origin.x] = shift;
        origin = newOrigin;
        field[origin.y][origin.x] = Directions::None;
        originObj.move(baseSize * sf::Vector2f(dirToCoords(shift)));
    }

    void buildPath() {

        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                //sf::Vector2f pathPosition = sf::Vector2f(offset) + sf::Vector2f(baseSize/2.0 + baseSize/30.0, baseSize/2.0 + baseSize / 30.0) + sf::Vector2f(j * baseSize, i * baseSize);
                sf::Vector2f pathPosition = sf::Vector2f(offset) + sf::Vector2f(baseSize / 2.0, baseSize / 2.0 ) + sf::Vector2f(j * baseSize, i * baseSize);
                sf::Vector2f pathSize;
                path[i * width + j].setOrigin(sf::Vector2f(baseSize / 30.0, baseSize / 30.0));
                if (field[i][j] == Directions::None) pathSize = sf::Vector2f(baseSize / 15.0, baseSize / 15.0);
                else
                {
                    pathSize = clamp(sf::Vector2f(baseSize * 16.0 / 15.0, baseSize / 15.0));
                    if (i == 0 && field[i][j] == Directions::Up ||
                        i == (height-1) && field[i][j] == Directions::Down ||
                        j == 0 && field[i][j] == Directions::Left ||
                        j == (width - 1) && field[i][j] == Directions::Right) 
                    {
                        pathSize = clamp(sf::Vector2f(baseSize * 8.0 / 15.0, baseSize / 15.0));
                        path[i * width + j].setFillColor(sf::Color::Red);
                    }

                        

                    path[i * width + j].setRotation(90.0 * int(field[i][j])); //поворачиваем полоску
                }
                path[i * width + j].setSize(pathSize);
                path[i * width + j].setPosition(pathPosition);
            }
        }
    }

    void buildWalls()
    {
        //сначала заполняем горизонтальные стены, потом вертикальные
        for (int i = 0; i < height+1; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                walls[i * width + j].setOrigin(sf::Vector2f(baseSize / 30.0, baseSize / 30.0));
                walls[i * width + j].setSize(clamp(sf::Vector2f(baseSize * 16.0 / 15.0, baseSize / 15.0)));
                walls[i * width + j].setPosition(sf::Vector2f(offset) + sf::Vector2f(j * baseSize, i * baseSize));
                //walls[i * width + j].setRotation(90.0 * int(field[i][j]));
                
                walls[i * width + j].setFillColor(sf::Color::Blue);
            }
        }
        int w0 = width * (height + 1);
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width+1; ++j)
            {
                walls[w0 + i * (width + 1) + j].setOrigin(sf::Vector2f(baseSize / 30.0, baseSize / 30.0));
                walls[w0 + i * (width + 1) + j].setSize(clamp(sf::Vector2f(baseSize * 16.0 / 15.0, baseSize / 15.0)));
                walls[w0 + i * (width + 1) + j].setPosition(sf::Vector2f(offset) + sf::Vector2f(j * baseSize, i * baseSize));
                walls[w0 + i * (width + 1) + j].setRotation(90.0);
                walls[w0 + i * (width + 1) + j].setFillColor(sf::Color::Blue);
            }
        }
        //убиираем лишние стены
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                switch (field[i][j])
                {
                case Directions::Right:
                    walls[w0 + i * (width + 1) + j + 1].setFillColor(sf::Color(255, 255, 255, 0));
                    break;
                case Directions::Left:
                    walls[w0 + i * (width + 1) + j].setFillColor(sf::Color(255, 255, 255, 0));
                    break;
                case Directions::Up:
                    walls[i * width + j].setFillColor(sf::Color(255, 255, 255, 0));
                    break;
                case Directions::Down:
                    walls[(i+1) * width + j].setFillColor(sf::Color(255, 255, 255, 0));
                    break;
                default:
                    break;
                }
            }
        }
    }

    vector<Directions> solve(sf::Vector2i pointA, sf::Vector2i pointB)
    {
        vector<Directions> result;
        sf::Vector2i pos = pointA;
        while (pos != origin)
        {
            result.push_back(field[pos.y][pos.x]);
            pos += dirToCoords(field[pos.y][pos.x]);
        }
        vector<Directions> temp;
        pos = pointB;
        while (pos != origin)
        {
            temp.push_back(field[pos.y][pos.x]);
            pos += dirToCoords(field[pos.y][pos.x]);
        }

        while (temp.size() > 0 && result.size() > 0 && temp.back() == result.back())
        {
            temp.pop_back();
            result.pop_back();
        }
        for (int i = temp.size() - 1; i >= 0; --i)
        {
            Directions reversedDir = Directions::None;
            switch (temp[i])
            {
            case Directions::Right:
                reversedDir = Directions::Left;
                break;
            case Directions::Left:
                reversedDir = Directions::Right;
                break;
            case Directions::Up:
                reversedDir = Directions::Down;
                break;
            case Directions::Down:
                reversedDir = Directions::Up;
                break;
            default:
                break;
            }
            result.push_back(reversedDir);
        }

        return result;
    }
    vector<sf::RectangleShape> buildRoute(sf::Vector2i start, vector<Directions> route) {
        vector<sf::RectangleShape> resRoute;
        sf::Vector2f edgeSize = clamp(sf::Vector2f(baseSize * 16.0 / 15.0, baseSize / 15.0));
        for (auto& dir : route) {
            resRoute.push_back(sf::RectangleShape(edgeSize));

            resRoute.back().setFillColor(sf::Color::Yellow);
            resRoute.back().setOrigin(sf::Vector2f(baseSize / 30.0, baseSize / 30.0));
            resRoute.back().setPosition(sf::Vector2f(offset) + sf::Vector2f(baseSize / 2.0, baseSize / 2.0) + baseSize * sf::Vector2f(start));
            resRoute.back().setRotation(90.0 * int(dir));

            start += dirToCoords(dir);
        }
        return resRoute;
    }
    void draw(sf::RenderWindow& window, bool drawPath = true)
    {
        if (drawPath)
            for (auto& fig : path)
                window.draw(fig);

        for (auto& fig : walls)
        {
            window.draw(fig);
        }
        window.draw(originObj);
        window.draw(finishObj);
    }


};

Matrix<float> prepareInput(const Maze& input, sf::Vector2i pos)
{
    Matrix<float> res(5*6*2+2);
    int k = 2;
    int w0 = input.width * (input.height + 1);
    for (int i = pos.x-2; i < pos.x+3; ++i)
    {
        for (int j = pos.y - 2; j < pos.y + 4; ++j)
        {
            if (i >= 0 && j >= 0 && i < input.width && j <= input.height
                && input.walls[j * input.width + i].getFillColor() != sf::Color(255, 255, 255, 0))
                res[k] = 1;
            k++;
            
        }
        
    }
    for (int i = pos.x - 2; i < pos.x + 4; ++i)
    {
        for (int j = pos.y - 2; j < pos.y + 3; ++j)
        {
            if (i >= 0 && j >= 0 && i <= input.width && j < input.height
                && input.walls[w0 + j * (input.width + 1) + i].getFillColor() != sf::Color(255, 255, 255, 0))
                res[k] = 1;
            k++;
        }

    }
    res[0] = pos.x;
    res[1] = pos.y;
    return res;
}

class Player
{
    sf::RectangleShape player;
    float stepSize = 0;
    Maze& maze;
public:
    sf::Vector2i pos = { 0, 0 };

    Player(Maze& maze): maze(maze)
    {
        setup();
    }
    void setup() {
        stepSize = maze.baseSize;
        player.setSize(sf::Vector2f(stepSize / 5, stepSize / 5));
        player.setOrigin(sf::Vector2f(stepSize / 10, stepSize / 10));
        player.setPosition(sf::Vector2f(maze.offset) + sf::Vector2f(stepSize / 2, stepSize / 2));

        player.setFillColor(sf::Color::Green);
    }
    bool step(Directions shift)
    {
        sf::Vector2i newPos = pos + dirToCoords(shift);
        sf::IntRect box(0, 0, maze.field[0].size(), maze.field.size());
        if (!box.contains(newPos)) return false;
        if (maze.field[pos.y][pos.x] == shift || dirToCoords(maze.field[newPos.y][newPos.x]) == -dirToCoords(shift))
        {
            player.move(stepSize * sf::Vector2f(dirToCoords(shift)));
            pos = newPos;
            return true;
        }
        else return false;
    }
    void resetPos(sf::Vector2i newPos = { 0, 0 })
    {
        player.move(stepSize * sf::Vector2f(newPos - pos));
        pos = newPos;
    }
    void draw(sf::RenderWindow& window) {
        window.draw(player);
    }

};

int main()
{
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Maze", sf::Style::Close);
    int width = 30, height = 30;
    sf::Clock timer;
    sf::Clock AItimer;
    EasyRandom posGenW(0, width-2);
    EasyRandom posGenH(0, height-2);
    Maze maze(width, height, window);
    Player player(maze);

    SimpleNN snn;
    snn.addLayer(5 * 6 * 2 + 2, 100);
    snn.addLayer(100, 30);
    snn.addLayer(30, 4);
    snn.check();
    cout << snn.forward(prepareInput(maze, player.pos)) << "\n";

    for (int i = 0; i < 1000; i++)
    {
        Matf ans(4);
        ans[int(maze.solve(player.pos, maze.finishPos)[0])] = 1;
        Matf X = prepareInput(maze, player.pos);
        
        grad cgrad = snn.emptyGrad();
        cgrad.dEdW.resize(snn.W.size());
        cgrad.dEdB.resize(snn.B.size());

        for (int j = 0; j < 10; ++j)
        {
            grad stepGrad = snn.trainStep(X, ans);
            maze.setup(width, height, window);
            player.resetPos({ posGenW.get(), posGenH.get() });
            cgrad += stepGrad;
        }
        cout << cgrad.loss / cgrad.cnt << "\n";
        snn.updateWeights(cgrad);
        
    }
    int cnt = 0;
    for (int i = 0; i < 1000; i++)
    {
        if (maze.solve(player.pos, maze.finishPos)[0] == Directions(snn.forward(prepareInput(maze, player.pos)).indexOfMax())) cnt++;
        maze.setup(width, height, window);
        player.resetPos({ posGenW.get(), posGenH.get() });
        
    }
    cout << cnt;
    window.setFramerateLimit(60);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            switch (event.key.code)
            {
            case sf::Keyboard::Escape:
                window.close();
                break;
            case sf::Keyboard::W:
                player.step(Directions::Up);
                break;
            case sf::Keyboard::A:
                player.step(Directions::Left);
                break;
            case sf::Keyboard::S:
                player.step(Directions::Down);
                break;
            case sf::Keyboard::D:
                player.step(Directions::Right);
                break;
            }
        }
        if (timer.getElapsedTime() > sf::milliseconds(50))
        {
            maze.originShift();
            maze.buildPath();
            maze.buildWalls();
            timer.restart();
        }
        if (AItimer.getElapsedTime() > sf::milliseconds(500))
        {
            player.step(Directions(snn.forward(prepareInput(maze, player.pos)).indexOfMax()));
            AItimer.restart();
        }
        if (player.pos == maze.finishPos)
        {
            if (height < width) height++;
            else width++;
            maze.setup(width, height, window);
            player.resetPos();
            player.setup();
        }

        window.clear();
        maze.draw(window, false);
        vector<sf::RectangleShape> route = maze.buildRoute(player.pos, maze.solve(player.pos, maze.finishPos));
        for (auto& shape : route) window.draw(shape);
        player.draw(window);

        window.display();
    }

    return 0;
}