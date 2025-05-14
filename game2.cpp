#include <cstdlib>
#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <ctime>
#include "network.h"
using namespace std;

#define DrawText DrawText
#define CloseWindow CloseWindow
#define ShowCursor ShowCursor

float screenWidth = 700;
float screenHeight = 670;
bool isPaused = false;
int isServer = false;
int runOnline = 0;
string mode = "Assets/";

struct state
{
    float x;
    float y;
    float p2;
    float p1;
};

enum THEMES
{
    THEME1,
    THEME2,
    THEME3,
    THEME4,
    THEME5,
    THEME6
};

float Clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

// below is for client

bool checkPause(bool isHover, Color *buttonColor, ENetPeer *peer, ENetHost *host)
{
    if (isHover)
    {
        *buttonColor = DARKGRAY;
    }
    else
    {
        *buttonColor = RAYWHITE;
    }
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        isPaused = !isPaused;
        sendPause(host, peer, isPaused);
    }
    return isPaused;
}

class scoreBoard
{
    int scoreLeft;
    int scoreRight;

public:
    scoreBoard()
    {
        scoreLeft = 0;
        scoreRight = 0;
    }
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
            scoreColor = (Color){120, 252, 255, 90};
            fontSize = 0.1625 * (screenWidth + screenHeight);
            break;
        case 2:
            posX = 0.1125 * screenWidth;
            posX2 = 0.82125 * screenWidth;
            posY = 0.4575 * screenHeight;
            scoreColor = (Color){150, 255, 255, 255};
            fontSize = 0.075 * (screenWidth + screenHeight);
            break;
        case 3:
            posX = 0.1125 * screenWidth;
            posX2 = 0.82125 * screenWidth;
            posY = 0.4575 * screenHeight;
            scoreColor = (Color){124, 252, 0, 255};
            fontSize = 0.075 * (screenWidth + screenHeight);
            break;
        case 4:
            posX = 0.175 * screenWidth;
            posX2 = 0.6875 * screenWidth;
            posY = 0.375 * screenHeight;
            scoreColor = (Color){255, 172, 28, 255};
            fontSize = 0.1625 * (screenWidth + screenHeight);
            break;
        default:
            posX = 0.175 * screenWidth;
            posX2 = 0.6875 * screenWidth;
            posY = 0.375 * screenHeight;
            scoreColor = (Color){120, 252, 255, 90};
            fontSize = 0.1625 * (screenWidth + screenHeight);
            break;
        }
        DrawText(TextFormat("%d", scoreRight), posX2, posY, fontSize, scoreColor);
        DrawText(TextFormat("%d", scoreLeft), posX, posY, fontSize, scoreColor);
    }
    int *getScore1()
    {
        return &scoreLeft;
    }
    int *getScore2()
    {
        return &scoreRight;
    }
};

class theme
{
    Color ballColor;
    Texture2D pi;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;

public:
    theme(Color b, Color ba, Color bo, Rectangle bou, int bw, string name)
    {
        ballColor = b;
        background = ba;
        border = bo;
        boundaries = bou;
        borderWidth = bw;

        Image bck = LoadImage(name.c_str());
        if (bck.data == NULL)
        {
            cout << "Failed to load background image!" << endl;
            exit(1);
        }
        ImageResize(&bck, bou.width, bou.height);
        pi = LoadTextureFromImage(bck);
        UnloadImage(bck);
    }
    void drawBoard()
    {
        ClearBackground(background);
        DrawRectangle(0, 0, screenWidth, 25, BLACK);
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawTexture(pi, boundaries.x, boundaries.y, WHITE);
        DrawCircleLines(boundaries.x + boundaries.width / 2, boundaries.y + boundaries.height / 2, 70, WHITE);
        DrawLine(screenWidth / 2, boundaries.y, screenWidth / 2, boundaries.y + boundaries.height, WHITE);
    }
    int getBorderWidth()
    {
        return borderWidth;
    }
    Color getBallColor()
    {
        return ballColor;
    }
    ~theme()
    {
        UnloadTexture(pi);
    }
};

class paddle
{
    int height;
    int width;
    int positionX;
    int positionY;
    Texture2D skin;
    Color color;

public:
    paddle(int x, int y, Color c, int h, int w, string name)
    {
        positionX = x;
        positionY = y;
        color = c;
        height = h;
        width = w;

        Image skinImg = LoadImage(name.c_str());
        if (skinImg.data == NULL)
        {
            cout << "Image was NULL" << endl;
            exit(1);
        }
        ImageResize(&skinImg, width, height);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    void setPositionY(int y)
    {
        positionY = y;
    }
    int getPositionY()
    {
        return positionY;
    }
    void drawpaddle()
    {
        DrawTexture(skin, positionX, positionY, WHITE);
    }
    void update()
    {
        if (IsKeyDown(KEY_W))
            positionY -= 5;
        if (IsKeyDown(KEY_S))
            positionY += 5;
        positionY = Clamp(positionY, 25.0f, 25.0f + screenHeight - height); // Adjusted clamping for top bar
    }
    Rectangle getRec()
    {
        Rectangle r = {(float)positionX, (float)positionY, (float)width, (float)height};
        return r;
    }
    ~paddle()
    {
        UnloadTexture(skin);
    }
};

class ball
{
    int positionX;
    int positionY;
    int radius;
    int ballSSpeedX;
    int ballSSpeedY;
    Color color;
    Texture2D skin;

public:
    ball(int x, int y, int r, Color c, int speedX, int speedY, string name)
    {
        positionX = x;
        positionY = y;
        radius = r;
        color = c;
        ballSSpeedX = speedX;
        ballSSpeedY = speedY;

        Image skinImg = LoadImage(name.c_str());
        if (skinImg.data == NULL)
        {
            cout << "Failed to load background image!" << endl;
            exit(1);
        }
        ImageResize(&skinImg, radius * 2, radius * 2);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    void drawball()
    {
        DrawTexture(skin, positionX - radius, positionY - radius, WHITE);
    }
    void setPositionX(int x)
    {
        positionX = x;
    }
    void setPositionY(int y)
    {
        positionY = y;
    }
    int getPositionX()
    {
        return positionX;
    }
    int getPositionY()
    {
        return positionY;
    }
    int getballSpeedX()
    {
        return ballSSpeedX;
    }
    int getballSpeedY()
    {
        return ballSSpeedY;
    }
    int getRadius()
    {
        return radius;
    }
    void update(Rectangle leftRec, Rectangle rightRec, int *score1, int *score2)
    {
        if (positionX + radius >= screenWidth || positionX - radius <= 0)
        {
            ballSSpeedX *= -1;
            if (positionX + radius >= screenWidth)
            {
                (*score2)++;
            }
            else
            {
                (*score1)++;
            }
        }
        if (positionY + radius >= screenHeight + 25 || positionY - radius <= 25)
        {
            ballSSpeedY *= -1;
        }
        if (CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
            CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, rightRec))
        {
            ballSSpeedX *= -1;
        }
        positionX += ballSSpeedX;
        positionY += ballSSpeedY;
    }
    ~ball()
    {
        UnloadTexture(skin);
    }
};

void winningPage(int winner)
{
    const char *path = NULL;
    switch (winner)
    {
    case 0:
        path = "Assets/winningPage/server.jpg";
        break;
    case 1:
        path = "Assets/winningPage/client.jpg";
        break;
    default:
        path = "Assets/winningPage/tie.jpg";
        break;
    }

    Texture2D image = LoadTexture(path);
    Rectangle dest = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
    Rectangle src = {0, 0, (float)image.width, (float)image.height};

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexturePro(image, src, dest, (Vector2){0, 0}, 0.0f, WHITE);

        EndDrawing();
    }
    UnloadTexture(image);
}

int runClient(THEMES currTheme, string ipInput)
{
    string ip = ipInput;
    int scoreChoice;
    string themePath;
    if (currTheme == THEME1)
    {
        themePath = "fireAndIce/";
        scoreChoice = 2;
    }
    if (currTheme == THEME2)
    {
        themePath = "forestAndWood/";
        scoreChoice = 3;
    }
    if (currTheme == THEME3)
    {
        themePath = "neon/";
        scoreChoice = 2;
    }
    if (currTheme == THEME4)
    {
        themePath = "futuristic/";
        scoreChoice = 2;
    }
    if (currTheme == THEME5)
    {
        themePath = "underWater/";
        scoreChoice = 1;
    }
    if (currTheme == THEME6)
    {
        themePath = "spacegalaxy/";
        scoreChoice = 1;
    }
    ENetHost *host = NULL;
    ENetPeer *peer = NULL;
    if (networkInitialize(MODE_CLIENT, ip.c_str(), &host, &peer) != 0)
    {
        printf("Failed to initialize network\n");
        return 1;
    }
    scoreBoard score;
    int oldSW = screenWidth, oldSH = screenHeight;
    state ballCState = {(float)screenWidth / 2, (float)screenHeight / 2, (float)screenHeight / 2, (float)screenHeight / 2};
    int signX = 1;
    int signY = 1;
    const double DELAY = 0.1;
    double now = GetTime();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "PONG BALL GAME MENU");
    SetTargetFPS(60);

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight}; // Play area starts at Y=25
    theme classic(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");

    paddle left(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    paddle right(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

    ball gameballC((ballCState.x * ((float)screenWidth / oldSW)), (ballCState.y * ((float)screenHeight / oldSH)),
                   (screenWidth * 0.02f), classic.getBallColor(),
                   (screenWidth * 0.007f), (screenHeight * 0.005f), mode + themePath + "ball.png");

    Color buttonColor = WHITE;
    int frameNo = 0;
    int scoreLeft = 0;
    int scoreRight = 0;
    Image paused = LoadImage("Assets/winningPage/paused.jpg");
    Texture2D pausedTexture = LoadTextureFromImage(paused);

    while (!WindowShouldClose())
    {
        // Handle window resize
        if (IsWindowResized() || !frameNo)
        {
            oldSW = screenWidth;
            oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
            int signSpeedX = (gameballC.getballSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameballC.getballSpeedY() < 0) ? -1 : 1;

            float newballCX = ballCState.x * ((float)screenWidth / oldSW);
            float newballCY = ballCState.y * ((float)screenHeight / oldSH);
            int newballCRadius = (int)(screenWidth * 0.02f);

            border = {0, 25, (float)screenWidth, (float)screenHeight};
            classic.~theme();
            gameballC.~ball();
            left.~paddle();
            right.~paddle();
            new (&left) paddle(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                               (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&right) paddle(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&gameballC) ball((int)newballCX, (int)newballCY, newballCRadius, classic.getBallColor(),
                                  (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + themePath + "ball.png");

            new (&classic) theme(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");
            networkReceiveScores(host, &scoreLeft, &scoreRight);
            *(score.getScore1()) = scoreLeft;
            *(score.getScore2()) = scoreRight;
        }

        // Pause button interaction
        Vector2 mousePos = GetMousePosition();
        Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
        bool isHover = CheckCollisionPointRec(mousePos, button);
        isPaused = checkPause(isHover, &buttonColor, peer, host);

        if (!isPaused)
        {
            if (frameNo % 2 == 0)
            {
                networkReceiveScores(host, &scoreLeft, &scoreRight);
                if (scoreLeft > 0 && scoreRight > 0 && scoreLeft < 50 && scoreRight < 50)
                {
                    *(score.getScore1()) = scoreLeft;
                    *(score.getScore2()) = scoreRight;
                }
            }
            if (frameNo % 5 == 0)
            {
                frameNo = receiveTime(host, (int)frameNo / 60);
                frameNo = frameNo * 60;
            }
            networkSendState(host, peer, 0, 0, 0, (float)right.getPositionY() / screenHeight);
            cout << "Sent as: " << (float)right.getPositionY() / screenHeight << endl;
            float dummy;
            networkReceiveState(host, &ballCState.x, &ballCState.y, &ballCState.p1, &dummy);
            cout << "Received: " << ballCState.x << " " << ballCState.y << " " << ballCState.p1 << "\n";

            ballCState.x *= screenWidth;
            ballCState.y *= screenHeight;
            ballCState.p1 *= screenHeight;

            if (!(std::isinf(ballCState.p1) || std::isnan(ballCState.p1)) &&
                !(std::isinf(ballCState.x) || std::isnan(ballCState.x)) &&
                !(std::isinf(ballCState.y) || std::isnan(ballCState.y)) &&
                ballCState.x > 0 && ballCState.x < screenWidth &&
                ballCState.y > 0 && ballCState.y < screenHeight &&
                ballCState.p1 >= 0 && ballCState.p1 < screenHeight)
            {
                left.setPositionY(ballCState.p1);
                gameballC.setPositionX(ballCState.x);
                gameballC.setPositionY(ballCState.y);
            }

            right.update();
            BeginDrawing();
            classic.drawBoard();
            score.drawBoard(scoreChoice);
            left.drawpaddle();
            right.drawpaddle();
            gameballC.drawball();

            DrawText(to_string((int)frameNo / 60).c_str(), 10, 5, 20, WHITE);
            DrawRectangleRec(button, buttonColor);
            DrawText("Pause", screenWidth - 65, 5, 15, WHITE);
            EndDrawing();
            frameNo++;
        }
        else
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(button, buttonColor);

            DrawTexturePro(
                pausedTexture,
                (Rectangle){0, 0, (float)pausedTexture.width, (float)pausedTexture.height},
                (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() - 30},
                (Vector2){0, 0},
                0.0f,
                WHITE);

            EndDrawing();
        }
        if ((int)frameNo / 60 >= 58)
            break;

        isPaused = receivePause(host, isPaused);
    }
    if (*(score.getScore1()) > *(score.getScore2()))
    {
        winningPage(0);
        CloseWindow();
        return 0;
    }
    else if (*(score.getScore1()) < *(score.getScore2()))
    {
        winningPage(1);
        CloseWindow();
        return 0;
    }
    else
    {
        winningPage(2);
        CloseWindow();
        return 0;
    }
    return 0;
}

int runServer(THEMES currTheme)
{
    int scoreChoice;
    string themePath;
    if (currTheme == THEME1)
    {
        themePath = "fireAndIce/";
        scoreChoice = 2;
    }
    if (currTheme == THEME2)
    {
        themePath = "forestAndWood/";
        scoreChoice = 1;
    }
    if (currTheme == THEME3)
    {
        themePath = "neon/";
        scoreChoice = 2;
    }
    if (currTheme == THEME4)
    {
        themePath = "futuristic/";
        scoreChoice = 2;
    }
    if (currTheme == THEME5)
    {
        themePath = "underWater/";
        scoreChoice = 3;
    }
    if (currTheme == THEME6)
    {
        themePath = "spacegalaxy/";
        scoreChoice = 1;
    }

    int oldSW = 800, oldSH = 770;
    int frameNo = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "PONG BALL GAME MENU");
    SetTargetFPS(60);

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight};
    theme classic(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");
    paddle left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    paddle right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    scoreBoard score;

    ball gameballS(oldSW / 2, oldSH / 2, (screenWidth * 0.02f), classic.getBallColor(), (screenWidth * 0.007f), (screenHeight * 0.005f), mode + themePath + "ball.png");
    state sts;
    state dummy = {0, 0, 0, 0};

    ENetHost *host = NULL;
    ENetPeer *peer = NULL;
    networkInitialize(MODE_SERVER, NULL, &host, &peer);
    sts.x = gameballS.getPositionX();
    sts.y = gameballS.getPositionY();
    sts.p1 = left.getPositionY();
    sts.p2 = right.getPositionY();

    Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
    Color buttonColor = WHITE;
    Image paused = LoadImage("Assets/winningPage/paused.jpg");
    Texture2D pausedTexture = LoadTextureFromImage(paused);

    while (!WindowShouldClose())
    {
        if (IsWindowResized() || !frameNo)
        {
            int oldSW = screenWidth;
            int oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
            button = {(float)screenWidth - 70, 5, 60, 15};
            int signSpeedX = (gameballS.getballSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameballS.getballSpeedY() < 0) ? -1 : 1;

            float newballSX = sts.x * ((float)screenWidth / oldSW);
            float newballSY = sts.y * ((float)screenHeight / oldSH);
            int newballSRadius = (int)(screenWidth * 0.02f);

            // Clamp ballS position within the new screen bounds.
            if (gameballS.getPositionX() < newballSRadius)
                gameballS.setPositionX(newballSRadius);
            if (gameballS.getPositionX() > screenWidth - newballSRadius)
                gameballS.setPositionX(screenWidth - newballSRadius);
            if (gameballS.getPositionY() < newballSRadius)
                gameballS.setPositionY(newballSRadius);
            if (gameballS.getPositionY() > screenHeight - 25 - newballSRadius)
                gameballS.setPositionY(screenHeight - 25 - newballSRadius);

            border = {0, 25, (float)screenWidth, (float)screenHeight};

            classic.~theme();
            gameballS.~ball();
            left.~paddle();
            right.~paddle();
            new (&left) paddle(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                               (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&right) paddle(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&gameballS) ball((int)newballSX, (int)newballSY, newballSRadius, classic.getBallColor(),
                                  (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + themePath + "ball.png");

            new (&classic) theme(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");
            networkSendScore(host, NULL, *(score.getScore1()), *(score.getScore2()));
        }

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPause(isHover, &buttonColor, peer, host);

        if (!isPaused)
        {
            sts.x = gameballS.getPositionX();
            sts.y = gameballS.getPositionY();
            sts.p1 = left.getPositionY();
            sts.p2 = right.getPositionY();

            networkProcessEvents(host);
            if (frameNo % 2 == 0)
            {
                networkSendScore(host, NULL, *(score.getScore1()), *(score.getScore2()));
            }
            if (frameNo % 5 == 0)
            {
                sendTime(host, (int)frameNo / 60);
            }
            networkReceiveState(host, &dummy.x, &dummy.y, &dummy.p1, &dummy.p2);
            cout << "Reaceived p2 as" << dummy.p2 << endl;
            dummy.p2 *= screenHeight;
            if (!((std::isinf(dummy.p2) || std::isnan(dummy.p2))) && dummy.p2 >= 0 && dummy.p2 <= screenHeight)
                right.setPositionY((int)dummy.p2);

            gameballS.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());
            left.update();

            networkSendState(host, NULL, (float)gameballS.getPositionX() / screenWidth,
                             (float)gameballS.getPositionY() / screenHeight, (float)left.getPositionY() / screenHeight, (float)right.getPositionY());

            BeginDrawing();
            classic.drawBoard();
            score.drawBoard(scoreChoice);
            left.drawpaddle();
            right.drawpaddle();
            gameballS.drawball();
            DrawRectangle(0, 0, screenWidth, 25, BLACK);
            DrawRectangleRec(button, buttonColor);
            DrawText(to_string((int)frameNo / 60).c_str(), 10, 5, 20, WHITE);
            DrawText("Pause", screenWidth - 65, 5, 15, WHITE);
            EndDrawing();
            frameNo++;
        }
        else
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(button, buttonColor);

            DrawTexturePro(
                pausedTexture,
                (Rectangle){0, 0, (float)pausedTexture.width, (float)pausedTexture.height - 30},
                (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() - 30},
                (Vector2){0, 0},
                0.0f,
                WHITE);

            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
        if ((int)frameNo / 60 >= 59)
        {
            break;
        }
    }
    UnloadTexture(pausedTexture);
    networkShutdown(host);
    if (*(score.getScore1()) > *(score.getScore2()))
    {
        winningPage(0);
        CloseWindow();
        return 0;
    }
    else if (*(score.getScore1()) < *(score.getScore2()))
    {
        winningPage(1);
        CloseWindow();
        return 0;
    }
    else
    {
        winningPage(2);
        CloseWindow();
        return 0;
    }
    return 0;
}

int main()
{
    int choice;
    cout << "Enter 1 for server and 0 for Client: \n";
    cin >> choice;
    int themeChoice;
    cout << "enter your theme choice\n1 for fire and Ice\n2 for forest and wood\n3 for neon\n4 for futuristic\n5 for underwater\n6 for spaceGalaxy\n";
    cin >> themeChoice;
    THEMES currentTheme;
    switch (themeChoice)
    {
    case 1:
        currentTheme = THEME1;
        break;
    case 2:
        currentTheme = THEME2;
        break;
    case 3:
        currentTheme = THEME3;
        break;
    case 4:
        currentTheme = THEME4;
        break;
    case 5:
        currentTheme = THEME5;
        break;
    case 6:
        currentTheme = THEME6;
        break;
    default:
        currentTheme = THEME3;
        break;
    }
    if (choice)
    {
        runServer(currentTheme);
    }
    else
    {
        string ipInput;
        cout << "\n Enter IP : ";
        cin >> ipInput;
        runClient(currentTheme, ipInput);
    }
    return 0;
}