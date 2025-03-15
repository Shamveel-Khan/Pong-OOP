#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "raylib.h"
#include "network.h"
#include <stdio.h>

#ifdef DrawText
    #undef DrawText
#endif
#ifdef CloseWindow
    #undef CloseWindow
#endif
#ifdef ShowCursor
    #undef ShowCursor
#endif

#define IS_SERVER 1

float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

class theme {
    Color ballColor;
    Texture2D pic;
    Color background;
    Color border;
    Rectangle boundaries;
    public:
        theme(Color b,Color ba,Color bo,Rectangle bou) {
            ballColor=b;
            border=bo;
            background=ba;
            boundaries = bou;
        }
        theme(Color b,Texture2D pi,Color ba,Color bo,Rectangle bou) {
            ballColor=b;
            pic=pi;
            background=ba;
            border=bo;
            boundaries = bou;
        }
        void drawBoard() {
            ClearBackground(background);
            DrawRectangleLinesEx(boundaries, 4, YELLOW);
            DrawCircleLines(300, 300, 70, WHITE);
            DrawLine(300, 0, 300, 600, WHITE);
        }
        Color getBallColor() {
            return ballColor;
        }
};

class paddle{
    int height;
    int width;
    int positionX;
    int positionY;
    Texture2D skin;
    Color color;
    public:
        paddle(int x,int y,Texture2D skin,Color c,int h,int w) {
            this->skin=skin;
            positionX=x;
            positionY=y;
            color=c;
            height=h;
            width=w;
        }
        paddle(int x,int y,Color c,int h,int w) {
            positionX=x;
            positionY=y;
            color=c;
            height=h;
            width=w;
        }
        void setPositionY(int y) {
            positionY=y;
        }
        int getPositionY() {
            return positionY;
        }
        void drawPaddle() {
            DrawRectangle(positionX, positionY, width, height, color);
        }
        void update() {
            if(positionX==10) {
                if(IsKeyDown(KEY_UP)) positionY -= 5;
                if(IsKeyDown(KEY_DOWN)) positionY += 5;
                positionY = Clamp(positionY, 0.0f, 500.0f);
            }
        }
        Rectangle getRec() {
            Rectangle r={(float)positionX,(float)positionY,(float)width,(float)height};
            return r;
        }
};

class ball {
    int positionX;
    int positionY;
    int radius;
    int ballSpeedX;
    int ballSpeedY;
    Color color;
    Texture2D skin;
    public:
        ball(int x,int y,int r,Color c,Texture2D t,int speedX,int speedY) {
            positionX=x;
            positionY=y;
            radius=r;
            color=c;
            skin=t;
            ballSpeedX=speedX;
            ballSpeedY=speedY;
        }
        ball(int x,int y,int r,Color c,int speedX,int speedY) {
            positionX=x;
            positionY=y;
            radius=r;
            color=c;
            ballSpeedX=speedX;
            ballSpeedY=speedY;
        }
        void drawBall() {
            DrawCircle(positionX, positionY, radius, color);
        }
        void setPositionX(int x) {
            positionX=x;
        }
        void setPositionY(int y) {
            positionY=y;
        }
        int getPositionX() {
            return positionX;
        }
        int getPositionY() {
            return positionY;
        }
        void update(Rectangle leftRec,Rectangle rightRec) {
            if(positionX+radius >= 600 || positionX-radius <= 0) ballSpeedX *= -1;
            if(positionY+radius >= 600 || positionY-radius <= 0) ballSpeedY *= -1;

            if(CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
                CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, rightRec)) {
            ballSpeedX *= -1;
            }

            positionX += ballSpeedX;
            positionY += ballSpeedY;
        }
};

struct state {
    float x;
    float y;
    float p1; 
    float p2; 
};

int main(void) {
    Color background={50, 168, 82,255};
    Rectangle border= {0,0,600,600};
    theme classic(RED,background,YELLOW,border);
    paddle left(10,300,WHITE,100,20);
    paddle right(570,300,WHITE,100,20);
    ball gameBall(300,300,20,classic.getBallColor(),7,5);
    state sts;
    state dummy = {0, 0, 0, 0};

    ENetHost* host = NULL;
    ENetPeer* peer = NULL;
    networkInitialize(MODE_SERVER, NULL, &host, &peer);
    
    sts.x = gameBall.getPositionX();
    sts.y = gameBall.getPositionY();
    sts.p1 = left.getPositionY(); 
    sts.p2 = right.getPositionY(); 

    InitWindow(600, 600, "Server - Multiplayer Pong");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        networkProcessEvents(host);

        networkReceiveState(host, &dummy.x, &dummy.y, &dummy.p1, &dummy.p2);
        right.setPositionY(dummy.p2);
        gameBall.update(left.getRec(),right.getRec());
        left.update();
        right.update();

        networkSendState(host, NULL, gameBall.getPositionX(), gameBall.getPositionY(),
         left.getPositionY(), right.getPositionY());

        BeginDrawing();
            classic.drawBoard();
            left.drawPaddle();
            right.drawPaddle();
            gameBall.drawBall();
        EndDrawing();
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}