#include <iostream>
#include <conio.h>
#include <time.h>
#include <windows.h>

using namespace std;
enum eDir { stop=0, LEFT=1, upleft=2, downleft=3, RIGHT=4, upright=5, downright=6};

class cBall
{
    private :
        int x,y;
        int origx, origy;
        eDir direction;
    public :
        cBall (int posx, int posy)
        {
            origx = x;
            origy = y;
            x = posx;
            y = posy;
            direction = stop;
        }
        void reset ()
        {
            x = origx;
            y = origy;
            direction = stop;
        }
        void changedir (eDir d)
        {
            direction =d;
        }
        void randomdir ()
        {
            direction = (eDir)((rand()%6)+1);
        }
        inline int getx() { return x;}
        inline int gety() { return y;}
        inline eDir getdir() {return direction;}
        void move()
        {

            switch(direction)
            {
                case LEFT:
                    x--;
                    break;
                case RIGHT:
                    x++;
                    break;
                case upleft:
                    x--; y++;
                    break;
                case downleft:
                    x--; y--;
                    break;
                case upright:
                    x++; y++;
                    break;
                case downright:
                    x--; y++;
                    break;
                case stop:
                    break;
            }
        }
        friend ostream & operator<<(ostream & o, cBall c)
        {
            o << "Ball [" << c.x << "," << c.y << "][" << c.direction << "]";
            return o;
        }
};

class Paddle
{
    private :
        int x,y;
        int origx, origy;
    public:
        Paddle()
        {
            x=y=0;
        }
        Paddle(int posx, int posy) : Paddle()
        {
            origx = posx;
            origy = posx;
            x = posx;
            y = posy;
        }
        inline void reset () { x=origx; y=origy;}
        inline int getx() { return x;}
        inline int gety() { return y;}
        inline void moveup() {y--;}
        inline void movedown() {y++;}
        friend ostream & operator<<(ostream & o, Paddle c)
        {
            o << "paddle [" << c.x << "," << c.y << "][";
            return o;
        }
};

void ClearScreen ()
{
    COORD cursorPosition; cursorPosition.X = 0;
    cursorPosition.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

class gamemanager
{
private:
    int width, height, score1, score2;
    bool quit;
    char up1, up2, down1, down2;
    cBall * ball;
    Paddle * p1;
    Paddle * p2;
public:
    gamemanager(int w, int h){
        srand(time(NULL));
        quit=false;
        up1='w';
        up2='i';
        down1='s';
        down2='k';
        score1=score2=0;
        width=w;
        height=h;
        ball = new cBall (w/2, h/2);
        p1 = new Paddle (1, h/2-3);
        p2 = new Paddle(w-2, h/2-3);
    }
    ~gamemanager()
    {
        delete ball, p1, p2;
    }
    void scoreup(Paddle *player)
    {
        if(player == p1)
            score1++;
        if(player == p2)
            score2++;
        ball->reset();
        p1->reset();
        p2->reset();
    }
    void draw()
    {
        ClearScreen();
        for (int i=0; i<width+2; i++)
            cout << "\xB2";
        cout << endl;

        for (int i=0; i<height; i++)
        {
            for (int j=0; j<width; j++)
            {
                int ballx = ball->getx();
                int bally = ball->gety();
                int player1x = p1->getx();
                int player2x = p2->getx();
                int player1y = p1->gety();
                int player2y = p2->gety();

                if(j==0)
                    cout << "\xB2";

                if(ballx == j && bally == i)
                    cout << "0";
                else if (player1x == j && player1y == i)
                    cout << "\xDB";
                else if (player1x == j && player1y+1 == i)
                    cout << "\xDB";
                else if (player1x == j && player1y+2 == i)
                    cout << "\xDB";
                else if (player1x == j && player1y+3 == i)
                    cout << "\xDB";

                else if (player2x == j && player2y == i)
                    cout << "\xDB";
                else if (player2x == j && player2y+1 == i)
                    cout << "\xDB";
                else if (player2x == j && player2y+2 == i)
                    cout << "\xDB";
                else if (player2x == j && player2y+3 == i)
                    cout << "\xDB";

                else
                    cout << " ";
                if(j == width-1)
                    cout << "\xB2";
            }
            cout << endl;
        }

    for (int i=0; i<width+2; i++)
        cout << "\xB2";
    cout << endl;
    cout << "Score 1: " << score1 << endl << "Score 2: " << score2 << endl;
    }

    void input()
    {
        ball->move();
        int ballx = ball->getx();
        int bally = ball->gety();
        int player1x = p1->getx();
        int player2x = p2->getx();
        int player1y = p1->gety();
        int player2y = p2->gety();

        if(_kbhit())
        {
            char current=_getch();
            if (current==up1)
                if(player1y>0)
                p1->moveup();
            if (current==up2)
                if(player2y>0)
                p2->moveup();
            if (current==down1)
                if(player1y+4<height)
                p1->movedown();
            if (current==down2)
                if(player2y+4<height)
                p2->movedown();
            if (ball->getdir()==stop)
                ball->randomdir();
            if (current == 'q')
                quit = true;
        }
    }

    void logic()
    {
        int ballx = ball->getx();
        int bally = ball->gety();
        int player1x = p1->getx();
        int player2x = p2->getx();
        int player1y = p1->gety();
        int player2y = p2->gety();

        for(int i=0; i<4; i++)
            if(ballx==player1x+1)
              if(bally==player1y+1)
                 ball->changedir((eDir)((rand()%3)+4));

        for(int i=0; i<4; i++)
            if(ballx==player2x+1)
              if(bally==player2y+1)
                 ball->changedir((eDir)((rand()%3)+1));

        if(bally==height-1)
            ball->changedir(ball->getdir()==downright? upright: upleft);
        if(bally==0)
            ball->changedir(ball->getdir()==upright? downright: downleft);
        if(ballx==0)
            scoreup(p2);
        if(ballx==width-1)
            scoreup(p1);
    }
void run()
{
    while(!quit)
    {
        draw();
        input();
        logic();
    }

}
};


int main()
{
    gamemanager c(40,20);
    c.run();
    return 0;
}
