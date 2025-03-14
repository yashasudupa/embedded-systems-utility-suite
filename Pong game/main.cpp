#include <iostream>
#include <conio.h>
#include <time.h>
#include <windows.h>

using namespace std;

// Enumeration for ball directions
enum eDir { stop=0, LEFT=1, upleft=2, downleft=3, RIGHT=4, upright=5, downright=6};

// Class representing the ball
class cBall {
private:
    int x, y;       // Current position of the ball
    int origx, origy; // Original position of the ball (reset point)
    eDir direction;  // Current direction of the ball

public:
    // Constructor to initialize ball's position
    cBall(int posx, int posy) {
        origx = posx;
        origy = posy;
        x = posx;
        y = posy;
        direction = stop;
    }
    
    // Resets the ball to its original position
    void reset() {
        x = origx;
        y = origy;
        direction = stop;
    }
    
    // Change the ball's direction
    void changedir(eDir d) {
        direction = d;
    }
    
    // Assigns a random direction to the ball
    void randomdir() {
        direction = (eDir)((rand() % 6) + 1);
    }
    
    // Getter methods
    inline int getx() { return x; }
    inline int gety() { return y; }
    inline eDir getdir() { return direction; }
    
    // Moves the ball based on its current direction
    void move() {
        switch (direction) {
            case LEFT: x--; break;
            case RIGHT: x++; break;
            case upleft: x--; y++; break;
            case downleft: x--; y--; break;
            case upright: x++; y++; break;
            case downright: x++; y--; break;
            case stop: break;
        }
    }
    
    // Overloading << operator to print ball info
    friend ostream & operator<<(ostream & o, cBall c) {
        o << "Ball [" << c.x << "," << c.y << "][" << c.direction << "]";
        return o;
    }
};

// Class representing the paddle
class Paddle {
private:
    int x, y;       // Current position of paddle
    int origx, origy; // Original position of paddle (reset point)

public:
    // Default constructor
    Paddle() { x = y = 0; }
    
    // Constructor to initialize paddle's position
    Paddle(int posx, int posy) : Paddle() {
        origx = posx;
        origy = posy;
        x = posx;
        y = posy;
    }
    
    // Resets the paddle to its original position
    inline void reset() { x = origx; y = origy; }
    
    // Getter methods
    inline int getx() { return x; }
    inline int gety() { return y; }
    
    // Moves paddle up and down
    inline void moveup() { y--; }
    inline void movedown() { y++; }
    
    // Overloading << operator to print paddle info
    friend ostream & operator<<(ostream & o, Paddle c) {
        o << "Paddle [" << c.x << "," << c.y << "]";
        return o;
    }
};

// Function to clear the console screen
void ClearScreen() {
    COORD cursorPosition;
    cursorPosition.X = 0;
    cursorPosition.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

// Class managing the game logic
class gamemanager {
private:
    int width, height; // Game area dimensions
    int score1, score2; // Player scores
    bool quit; // Flag to exit game
    char up1, up2, down1, down2; // Player controls
    cBall *ball; // Ball object
    Paddle *p1, *p2; // Paddle objects

public:
    // Constructor to initialize game parameters
    gamemanager(int w, int h) {
        srand(time(NULL));
        quit = false;
        up1 = 'w'; up2 = 'i';
        down1 = 's'; down2 = 'k';
        score1 = score2 = 0;
        width = w;
        height = h;
        ball = new cBall(w / 2, h / 2);
        p1 = new Paddle(1, h / 2 - 3);
        p2 = new Paddle(w - 2, h / 2 - 3);
    }
    
    // Destructor to free allocated memory
    ~gamemanager() {
        delete ball, p1, p2;
    }
    
    // Increases the score for a player and resets ball and paddles
    void scoreup(Paddle *player) {
        if (player == p1) score1++;
        if (player == p2) score2++;
        ball->reset();
        p1->reset();
        p2->reset();
    }
    
    // Draws the game state in console
    void draw() {
        ClearScreen();
        
        for (int i = 0; i < width + 2; i++) cout << "\xB2";
        cout << endl;
        
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j == 0) cout << "\xB2";
                if (ball->getx() == j && ball->gety() == i) cout << "0";
                else if (p1->getx() == j && (i >= p1->gety() && i < p1->gety() + 4)) cout << "\xDB";
                else if (p2->getx() == j && (i >= p2->gety() && i < p2->gety() + 4)) cout << "\xDB";
                else cout << " ";
                if (j == width - 1) cout << "\xB2";
            }
            cout << endl;
        }
        for (int i = 0; i < width + 2; i++) cout << "\xB2";
        cout << endl;
        cout << "Score 1: " << score1 << " Score 2: " << score2 << endl;
    }
    
    // Handles user input for paddle movement and game controls
    void input() {
        ball->move();
        if (_kbhit()) {
            char current = _getch();
            if (current == up1 && p1->gety() > 0) p1->moveup();
            if (current == up2 && p2->gety() > 0) p2->moveup();
            if (current == down1 && p1->gety() + 4 < height) p1->movedown();
            if (current == down2 && p2->gety() + 4 < height) p2->movedown();
            if (ball->getdir() == stop) ball->randomdir();
            if (current == 'q') quit = true;
        }
    }
    
    // Runs the game loop
    void run() {
        while (!quit) {
            draw();
            input();
        }
    }
};

int main() {
    gamemanager c(40, 20);
    c.run();
    return 0;
}
