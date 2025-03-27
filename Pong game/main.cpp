#include <iostream>
#include <vector>
#include <memory>
#include <conio.h>
#include <chrono>
#include <thread>

using namespace std;

enum class Direction { STOP = 0, LEFT, UPLEFT, DOWNLEFT, RIGHT, UPRIGHT, DOWNRIGHT };

class Ball {
private:
    int x, y;
    int origX, origY;
    Direction dir;

public:
    Ball(int posX, int posY) : x(posX), y(posY), origX(posX), origY(posY), dir(Direction::STOP) {}

    void reset() { x = origX; y = origY; dir = Direction::STOP; }
    void setDirection(Direction d) { dir = d; }
    void randomDirection() { dir = static_cast<Direction>((rand() % 6) + 1); }

    int getX() const { return x; }
    int getY() const { return y; }
    Direction getDirection() const { return dir; }

    void move() {
        switch (dir) {
            case Direction::LEFT: x--; break;
            case Direction::RIGHT: x++; break;
            case Direction::UPLEFT: x--; y--; break;
            case Direction::DOWNLEFT: x--; y++; break;
            case Direction::UPRIGHT: x++; y--; break;
            case Direction::DOWNRIGHT: x++; y++; break;
            default: break;
        }
    }
};

class Paddle {
private:
    int x, y;
    int origX, origY;
    static constexpr int paddleHeight = 4;

public:
    Paddle(int posX, int posY) : x(posX), y(posY), origX(posX), origY(posY) {}
    void reset() { x = origX; y = origY; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getHeight() const { return paddleHeight; }
    void moveUp() { if (y > 0) y--; }
    void moveDown(int height) { if (y + paddleHeight < height) y++; }
};

class GameManager {
private:
    int width, height;
    int score1, score2;
    bool quit;
    char up1, up2, down1, down2;
    unique_ptr<Ball> ball;
    unique_ptr<Paddle> p1, p2;

public:
    GameManager(int w, int h)
        : width(w), height(h), score1(0), score2(0), quit(false),
          up1('w'), up2('i'), down1('s'), down2('k'),
          ball(make_unique<Ball>(w / 2, h / 2)),
          p1(make_unique<Paddle>(1, h / 2 - 2)),
          p2(make_unique<Paddle>(w - 2, h / 2 - 2)) {}

    void scoreUp(const Paddle* player) {
        if (player == p1.get()) score1++;
        else if (player == p2.get()) score2++;
        ball->reset();
        p1->reset();
        p2->reset();
    }

    void draw() const {
        system("cls"); // Clear console
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j == 0 || j == width - 1) cout << "#";
                else if (ball->getX() == j && ball->getY() == i) cout << "O";
                else if (p1->getX() == j && i >= p1->getY() && i < p1->getY() + p1->getHeight()) cout << "|";
                else if (p2->getX() == j && i >= p2->getY() && i < p2->getY() + p2->getHeight()) cout << "|";
                else cout << " ";
            }
            cout << endl;
        }

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;
        cout << "Score 1: " << score1 << " Score 2: " << score2 << endl;
    }

    void input() {
        ball->move();
        if (_kbhit()) {
            char key = _getch();
            if (key == up1) p1->moveUp();
            if (key == up2) p2->moveUp();
            if (key == down1) p1->moveDown(height);
            if (key == down2) p2->moveDown(height);
            if (ball->getDirection() == Direction::STOP) ball->randomDirection();
            if (key == 'q') quit = true;
        }
    }

    void run() {
        while (!quit) {
            draw();
            input();
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
};

int main() {
    GameManager game(40, 20);
    game.run();
    return 0;
}
