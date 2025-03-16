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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define IS_CLIENT 1

int screenHeight=600,screenWidth=600;

struct state {
    float x;
    float y;
    float p2;    
    float p1; 
};

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
    int borderWidth;
public:
    theme(Color b, Color ba, Color bo, Rectangle bou, int bw) {
        ballColor = b;
        border = bo;
        background = ba;
        boundaries = bou;
        borderWidth = bw;
    }
    theme(Color b, Texture2D pi, Color ba, Color bo, Rectangle bou, int bw) {
        ballColor = b;
        pic = pi;
        background = ba;
        border = bo;
        boundaries = bou;
        borderWidth = bw;
    }
    void drawBoard() {
        ClearBackground(background);
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawCircleLines(boundaries.width/2, boundaries.height/2, 70, WHITE);
        DrawLine(screenWidth/2, 0, screenWidth/2, screenHeight, WHITE);
    }
    int getBorderWidth() {
        return borderWidth;
    }
    Color getBallColor() {
        return ballColor;
    }
};

class paddle {
    int height;
    int width;
    int positionX;
    int positionY;
    Texture2D skin;
    Color color;
public:
    paddle(int x, int y, Texture2D skin, Color c, int h, int w) {
        this->skin = skin;
        positionX = x;
        positionY = y;
        color = c;
        height = h;
        width = w;
    }
    paddle(int x, int y, Color c, int h, int w) {
        positionX = x;
        positionY = y;
        color = c;
        height = h;
        width = w;
    }
    void setPositionY(int y) {
        positionY = y;
    }
    int getPositionY() {
        return positionY;
    }
    void drawPaddle() {
        DrawRectangle(positionX, positionY, width, height, color);
    }
    void update() {
        if(positionX == 570) {
            if(IsKeyDown(KEY_W)) positionY -= 5;
            if(IsKeyDown(KEY_S)) positionY += 5;
            positionY = Clamp(positionY, 0.0f, 500.0f);
        }
    }
    Rectangle getRec() {
        Rectangle r = {(float)positionX, (float)positionY, (float)width, (float)height};
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
    ball(int x, int y, int r, Color c, Texture2D t, int speedX, int speedY) {
        positionX = x;
        positionY = y;
        radius = r;
        color = c;
        skin = t;
        ballSpeedX = speedX;
        ballSpeedY = speedY;
    }
    ball(int x, int y, int r, Color c, int speedX, int speedY) {
        positionX = x;
        positionY = y;
        radius = r;
        color = c;
        ballSpeedX = speedX;
        ballSpeedY = speedY;
    }
    void drawBall() {
        DrawCircle(positionX, positionY, radius, color);
    }
    void setPositionX(int x) {
        positionX = x;
    }
    void setPositionY(int y) {
        positionY = y;
    }
    int getPositionX() {
        return positionX;
    }
    int getPositionY() {
        return positionY;
    }
    void update(Rectangle leftRec, Rectangle rightRec) {
        if(positionX + radius >= screenWidth || positionX - radius <= 0) ballSpeedX *= -1;
        if(positionY + radius >= screenHeight || positionY - radius <= 0) ballSpeedY *= -1;
        if(CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
           CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, rightRec)) {
            ballSpeedX *= -1;
        }
        positionX += ballSpeedX;
        positionY += ballSpeedY;
    }
};


int main(void) {
    Color background = {50, 168, 82, 255};
    Rectangle border = {0, 0, (float)screenWidth, (float)screenHeight};
    theme classic(RED, background, YELLOW, border, 5);
    paddle left(classic.getBorderWidth()+5, screenHeight/2, WHITE, 100, 20);
    paddle right(screenWidth-classic.getBorderWidth()-25, screenHeight/2, WHITE, 100, 20);
    ball gameBall(screenWidth/2, screenHeight/2, 20, classic.getBallColor(), 7, 5);

    ENetHost* host = NULL;
    ENetPeer* peer = NULL;
    
    if(networkInitialize(MODE_CLIENT, "192.168.119.197", &host, &peer) != 0) {
        printf("Failed to initialize network\n");
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Client - Multiplayer Pong");
    SetTargetFPS(60);
    
    state ballState = {(float)screenWidth/2, (float)screenHeight/2, (float)screenHeight/2, (float)screenHeight/2};

    while(!WindowShouldClose()) {
        
        // RESPONSIVE CODE: Handle window resize (same as server)
        if (IsWindowResized()) {
            int oldSW = screenWidth;
            int oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();

            left = paddle(classic.getBorderWidth()+5, screenHeight/2 - (int)(screenHeight * 0.165f / 2), WHITE,
                          (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));
            right = paddle(screenWidth - 30, screenHeight/2 - (int)(screenHeight * 0.165f / 2), WHITE,
                           (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));

            float newBallX = ballState.x * ((float)screenWidth / oldSW);
            float newBallY = ballState.y * ((float)screenHeight / oldSH);
            int newBallRadius = (int)(screenWidth * 0.02f);
            gameBall = ball((int)newBallX, (int)newBallY, newBallRadius, classic.getBallColor(),
                            (int)(screenWidth * 0.007f), (int)(screenHeight * 0.005f));

            border = {0, 0, (float)screenWidth, (float)screenHeight};
            classic = theme(RED, background, YELLOW, border, 5);
        }
        
        networkSendState(host, peer, 0, 0, 0, right.getPositionY());

        float dummy;
        networkReceiveState(host, &ballState.x, &ballState.y, &ballState.p1, &dummy);
        left.setPositionY(ballState.p1);
        right.update();
        gameBall.setPositionX((int)ballState.x);
        gameBall.setPositionY((int)ballState.y);
        
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
