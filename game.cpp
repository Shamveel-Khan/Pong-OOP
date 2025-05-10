#include "raylib.h"
#include "network.h"
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <cmath>
#include <iostream>
using namespace std;

#define WIN32_LEAN_AND_MEAN

#ifdef DrawText
#undef DrawText
#endif
#ifdef CloseWindow
#undef CloseWindow
#endif
#ifdef ShowCursor
#undef ShowCursor
#endif

int screenWidth = 700;
int screenHeight = 570;
bool isPaused = false;
string mode = "Assets/";

struct state
{
    float x;
    float y;
    float p1;
    float p2;
};

float Clamp(float value, float min, float max)
{
    return (value < min) ? min : (value > max) ? max
                                               : value;
}

class scoreBoard
{
    int scoreLeft;
    int scoreRight;

public:
    scoreBoard() : scoreLeft(0), scoreRight(0) {}

    void drawBoard(int choice)
    {
        float posX, posX2, posY, fontSize;
        Color scoreColor;
        switch (choice)
        {
        case 1:
            posX = 0.175 * screenWidth;
            posX2 = 0.6875 * screenWidth;
            posY = 0.375 * screenHeight;
            scoreColor = {120, 252, 255, 90};
            fontSize = 0.1625 * (screenWidth + screenHeight);
            break;
        case 2:
            posX = 0.1125 * screenWidth;
            posX2 = 0.82125 * screenWidth;
            posY = 0.4575 * screenHeight;
            scoreColor = {150, 255, 255, 255};
            fontSize = 0.075 * (screenWidth + screenHeight);
            break;
        case 3:
            posX = 0.1125 * screenWidth;
            posX2 = 0.82125 * screenWidth;
            posY = 0.4575 * screenHeight;
            scoreColor = {124, 252, 0, 255};
            fontSize = 0.075 * (screenWidth + screenHeight);
            break;
        default:
            posX = 0.175 * screenWidth;
            posX2 = 0.6875 * screenWidth;
            posY = 0.375 * screenHeight;
            scoreColor = {120, 252, 255, 90};
            fontSize = 0.1625 * (screenWidth + screenHeight);
            break;
        }
        DrawText(TextFormat("%d", scoreRight), posX2, posY, fontSize, scoreColor);
        DrawText(TextFormat("%d", scoreLeft), posX, posY, fontSize, scoreColor);
    }

    int *getScore1() { return &scoreLeft; }
    int *getScore2() { return &scoreRight; }
};

class theme
{
    Color ballColor;
    Texture2D pic;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;

public:
    theme(Color b, Color ba, Color bo, Rectangle bou, int bw, string name) : ballColor(b), background(ba), border(bo), boundaries(bou), borderWidth(bw)
    {
        Image bck = LoadImage(name.c_str());
        ImageResize(&bck, bou.width, bou.height);
        pic = LoadTextureFromImage(bck);
        UnloadImage(bck);
    }

    void drawBoard(int sw, int sh)
    {
        ClearBackground(background);
        DrawRectangle(0, 0, screenWidth, 25, BLACK);
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawTexture(pic, boundaries.x, boundaries.y, WHITE);
        DrawCircleLines(boundaries.x + boundaries.width / 2, boundaries.y + boundaries.height / 2, 70, WHITE);
        DrawLine(screenWidth / 2, boundaries.y, screenWidth / 2, boundaries.y + boundaries.height, WHITE);
    }

    int getBorderWidth() { return borderWidth; }
    Color getBallColor() { return ballColor; }
    ~theme() { UnloadTexture(pic); }
};

class paddle
{
    int height, width, positionX, positionY;
    Texture2D skin;
    Color color;

public:
    paddle(int x, int y, Color c, int h, int w, string name) : positionX(x), positionY(y), color(c), height(h), width(w)
    {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, width, height);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }

    void setPositionY(int y) { positionY = y; }
    int getPositionY() { return positionY; }
    void drawPaddle() { DrawTexture(skin, positionX, positionY, WHITE); }

    void update()
    {
        if (IsKeyDown(KEY_W))
            positionY -= 5;
        if (IsKeyDown(KEY_S))
            positionY += 5;
        positionY = Clamp(positionY, 25.0f, (float)screenHeight - height + 25);
    }

    Rectangle getRec() { return {(float)positionX, (float)positionY, (float)width, (float)height}; }
    ~paddle() { UnloadTexture(skin); }
};

class ball
{
    int positionX, positionY, radius, ballSpeedX, ballSpeedY;
    Color color;
    Texture2D skin;

public:
    ball(int x, int y, int r, Color c, int speedX, int speedY, string name) : positionX(x), positionY(y), radius(r), color(c), ballSpeedX(speedX), ballSpeedY(speedY)
    {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, radius * 2, radius * 2);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }

    void drawBall() { DrawTexture(skin, positionX - radius, positionY - radius, WHITE); }
    void setPositionX(int x) { positionX = x; }
    void setPositionY(int y) { positionY = y; }
    int getPositionX() { return positionX; }
    int getPositionY() { return positionY; }
    int getBallSpeedX() { return ballSpeedX; }
    int getBallSpeedY() { return ballSpeedY; }

    void update(Rectangle leftRec, Rectangle rightRec, int *score1, int *score2)
    {
        if (positionX + radius >= screenWidth || positionX - radius <= 0)
        {
            ballSpeedX *= -1;
            (positionX + radius >= screenWidth) ? (*score2)++ : (*score1)++;
        }
        if (positionY + radius >= screenHeight + 25 || positionY - radius <= 25)
            ballSpeedY *= -1;

        if (CheckCollisionCircleRec({(float)positionX, (float)positionY}, radius, leftRec) ||
            CheckCollisionCircleRec({(float)positionX, (float)positionY}, radius, rightRec))
            ballSpeedX *= -1;

        positionX += ballSpeedX;
        positionY += ballSpeedY;
    }

    ~ball() { UnloadTexture(skin); }
};

bool checkPause(bool isHover, Color *buttonColor, ENetHost *host, ENetPeer *peer = nullptr)
{
    if (isHover)
        *buttonColor = DARKGRAY;
    else
        *buttonColor = WHITE;

    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        isPaused = !isPaused;
        if (peer)
            sendPause(host, peer, isPaused);
        else
            sendPause(host, nullptr, isPaused);
    }
    return isPaused;
}

int runServer()
{
    int choice;
    cout << "Enter theme choice:\n1. Underwater\n2. Fire & Ice\n3. Forest\n4. Neon\n5. Futuristic\n6. Space\n";
    cin >> choice;
    string mode2 = "underWater/";
    switch (choice)
    {
    case 2:
        mode2 = "fireAndIce/";
        break;
    case 3:
        mode2 = "forestAndWood/";
        break;
    case 4:
        mode2 = "neon/";
        break;
    case 5:
        mode2 = "futuristic/";
        break;
    case 6:
        mode2 = "spacegalaxy/";
        break;
    }

    InitWindow(screenWidth, screenHeight + 30, "Server - Pong");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight};
    theme classic(RED, BLACK, YELLOW, border, 5, mode + mode2 + "background.png");
    paddle left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    paddle right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    scoreBoard score;

    ball gameBall(screenWidth / 2, screenHeight / 2, screenWidth * 0.02f, classic.getBallColor(),
                  screenWidth * 0.007f, screenHeight * 0.005f, mode + mode2 + "ball.png");

    ENetHost *host = nullptr;
    ENetPeer *peer = nullptr;
    networkInitialize(MODE_SERVER, nullptr, &host, &peer);

    Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
    Color buttonColor = WHITE;

    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
            border = {0, 25, (float)screenWidth, (float)screenHeight};
            classic.~theme();
            new (&classic) theme(RED, BLACK, YELLOW, border, 5, mode + mode2 + "background.png");
        }

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPause(isHover, &buttonColor, host);

        if (!isPaused)
        {
            state sts = {(float)gameBall.getPositionX(), (float)gameBall.getPositionY(),
                         (float)left.getPositionY(), (float)right.getPositionY()};

            state dummy = {0};
            networkProcessEvents(host);
            networkReceiveState(host, &dummy.x, &dummy.y, &dummy.p1, &dummy.p2);
            right.setPositionY(Clamp(dummy.p2 * screenHeight, 25.0f, (float)screenHeight - left.getRec().height + 25));

            gameBall.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());
            left.update();

            networkSendState(host, nullptr, gameBall.getPositionX() / (float)screenWidth,
                             gameBall.getPositionY() / (float)screenHeight,
                             left.getPositionY() / (float)screenHeight,
                             right.getPositionY() / (float)screenHeight);

            BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(choice);
            left.drawPaddle();
            right.drawPaddle();
            gameBall.drawBall();
            EndDrawing();
        }
        else
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("PAUSED", screenWidth / 2 - 40, screenHeight / 2, 20, WHITE);
            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}

int runClient()
{
    cout << "Enter server IP: ";
    string ip;
    cin >> ip;

    int choice;
    cout << "Enter theme choice:\n1. Underwater\n2. Fire & Ice\n3. Forest\n4. Neon\n5. Futuristic\n6. Space\n";
    cin >> choice;
    string mode2 = "underWater/";
    switch (choice)
    {
    case 2:
        mode2 = "fireAndIce/";
        break;
    case 3:
        mode2 = "forestAndWood/";
        break;
    case 4:
        mode2 = "neon/";
        break;
    case 5:
        mode2 = "futuristic/";
        break;
    case 6:
        mode2 = "spacegalaxy/";
        break;
    }

    InitWindow(screenWidth, screenHeight + 30, "Client - Pong");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    ENetHost *host = nullptr;
    ENetPeer *peer = nullptr;
    if (networkInitialize(MODE_CLIENT, ip.c_str(), &host, &peer) != 0)
        return 1;

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight};
    theme classic(RED, {50, 168, 82, 255}, YELLOW, border, 5, mode + mode2 + "background.png");
    paddle left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    paddle right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    scoreBoard score;

    ball gameBall(screenWidth / 2, screenHeight / 2, screenWidth * 0.02f, classic.getBallColor(),
                  screenWidth * 0.007f, screenHeight * 0.005f, mode + mode2 + "ball.png");

    Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
    Color buttonColor = WHITE;

    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
            border = {0, 25, (float)screenWidth, (float)screenHeight};
            classic.~theme();
            new (&classic) theme(RED, {50, 168, 82, 255}, YELLOW, border, 5, mode + mode2 + "background.png");
        }

        Vector2 mousePos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(mousePos, button);
        isPaused = checkPause(isHover, &buttonColor, host, peer);

        if (!isPaused)
        {
            networkSendState(host, peer, 0, 0, 0, right.getPositionY() / (float)screenHeight);

            state ballState = {0};
            float dummy;
            networkReceiveState(host, &ballState.x, &ballState.y, &ballState.p1, &dummy);

            left.setPositionY(Clamp(ballState.p1 * screenHeight, 25.0f,
                                    (float)screenHeight - left.getRec().height + 25));
            gameBall.setPositionX(ballState.x * screenWidth);
            gameBall.setPositionY(ballState.y * screenHeight);

            right.update();

            BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(choice);
            left.drawPaddle();
            right.drawPaddle();
            gameBall.drawBall();
            EndDrawing();
        }
        else
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("PAUSED", screenWidth / 2 - 40, screenHeight / 2, 20, WHITE);
            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}

int main()
{
    cout << "Run as:\n1. Server\n2. Client\n";
    int choice;
    cin >> choice;
    if (choice == 1)
        runServer();
    else
        runClient();
    return 0;
}