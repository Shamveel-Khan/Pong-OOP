#include "raylib.h"
#include "network.h"
#include <stdio.h>
#include <math.h>    

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef DrawText
    #undef DrawText
#endif
#ifdef CloseWindow
    #undef CloseWindow
#endif
#ifdef ShowCursor
    #undef ShowCursor
#endif

int screenWidth = 600;
int screenHeight = 600;

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
    void drawBoard(int sw, int sh) {
        ClearBackground(background);
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawCircleLines(boundaries.width / 2, boundaries.height / 2, 70, WHITE);
        DrawLine(boundaries.width / 2, 0, boundaries.width / 2, boundaries.height, WHITE);
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
        if (positionX == 10) {
            if (IsKeyDown(KEY_W)) positionY -= screenHeight * 0.01f;
            if (IsKeyDown(KEY_S)) positionY += screenHeight * 0.01f;
            positionY = (int)Clamp(positionY, 0.0f, (float)screenHeight - height);
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
    int getRadius() {
        return radius;
    }
    void update(Rectangle leftRec, Rectangle rightRec) {
        if (positionX + radius >= screenWidth || positionX - radius <= 0) ballSpeedX *= -1;
        if (positionY + radius >= screenHeight || positionY - radius <= 0) ballSpeedY *= -1;
        if (CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
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
    int oldSW=600,oldSH=600;
    Color background = {50, 168, 82, 255};
    Rectangle border = {0, 0, (float)screenWidth, (float)screenHeight};
    theme classic(RED, background, YELLOW, border, 5);
    paddle left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));
    paddle right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));

    ball gameBall(oldSW/2, oldSH/2, (int)(screenWidth * 0.02f), classic.getBallColor(), (int)(screenWidth * 0.007f), (int)(screenHeight * 0.005f));    
    state sts;
    state dummy = {0, 0, 0, 0};

    ENetHost* host = NULL;
    ENetPeer* peer = NULL;
    networkInitialize(MODE_SERVER, NULL, &host, &peer);

    sts.x = gameBall.getPositionX();
    sts.y = gameBall.getPositionY();
    sts.p1 = left.getPositionY();
    sts.p2 = right.getPositionY();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Server - Multiplayer Pong");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            int oldSW = screenWidth;
            int oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();

            left = paddle(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));
            right = paddle(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));

            float newBallX = sts.x * ((float)screenWidth / oldSW);
            float newBallY = sts.y * ((float)screenHeight / oldSH);
            int newBallRadius = (int)(screenWidth * 0.02f);
            gameBall = ball((int)newBallX, (int)newBallY, newBallRadius, classic.getBallColor(), (int)(screenWidth * 0.007f), (int)(screenHeight * 0.005f));

            // Clamp ball position within the new screen bounds.
            if (gameBall.getPositionX() < newBallRadius) gameBall.setPositionX(newBallRadius);
            if (gameBall.getPositionX() > screenWidth - newBallRadius) gameBall.setPositionX(screenWidth - newBallRadius);
            if (gameBall.getPositionY() < newBallRadius) gameBall.setPositionY(newBallRadius);
            if (gameBall.getPositionY() > screenHeight - newBallRadius) gameBall.setPositionY(screenHeight - newBallRadius);

            // Update the border and re-create the theme with the new board dimensions.
            border = {0, 0, (float)screenWidth, (float)screenHeight};
            classic = theme(RED, background, YELLOW, border, 5);
        }

        // Update current state (used for networking position scaling).
        sts.x = gameBall.getPositionX();
        sts.y = gameBall.getPositionY();
        sts.p1 = left.getPositionY();
        sts.p2 = right.getPositionY();

        networkProcessEvents(host);
        networkReceiveState(host, &dummy.x, &dummy.y, &dummy.p1, &dummy.p2);
        right.setPositionY((int)dummy.p2);
        gameBall.update(left.getRec(), right.getRec());
        left.update();
        right.update();

        networkSendState(host, NULL, gameBall.getPositionX(), gameBall.getPositionY(),
                         left.getPositionY(), right.getPositionY());

        BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            left.drawPaddle();
            right.drawPaddle();
            gameBall.drawBall();
        EndDrawing();
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}
