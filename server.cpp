#include "raylib.h"
#include "network.h"
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <cmath>
#include <iostream>
using namespace std;

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

int screenWidth = 700;
int screenHeight = 570;
bool isPaused = false;
string mode = "Assets/";

float Clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

bool checkPause(bool isHover, Color *buttonColor, ENetHost *host)
{
    if (isHover)
    {
        *buttonColor = DARKGRAY;
    }
    else
    {
        *buttonColor = WHITE;
    }
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        isPaused = !isPaused;
        sendPause(host, NULL, isPaused);
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
    Texture2D pic;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;

public:
    theme(Color b, Color ba, Color bo, Rectangle bou, int bw, string name)
    {
        ballColor = b;
        border = bo;
        background = ba;
        boundaries = bou;
        borderWidth = bw;

        Image bck = LoadImage(name.c_str());
        if (bck.data == NULL)
        {
            cout << "Failed to load background image!" << endl;
            exit(1);
        }
        ImageResize(&bck, bou.width, bou.height);
        pic = LoadTextureFromImage(bck);
        UnloadImage(bck);
    }
    void drawBoard(int sw, int sh)
    {
        ClearBackground(background);
        DrawRectangle(0, 0, screenWidth, 25, BLACK); // Top bar
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawTexture(pic, boundaries.x, boundaries.y, WHITE);
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
        UnloadTexture(pic);
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
    void drawPaddle()
    {
        DrawTexture(skin, positionX, positionY, WHITE);
    }
    void update()
    {
        if (positionX == 10)
        {
            if (IsKeyDown(KEY_W))
                positionY -= screenHeight * 0.01f;
            if (IsKeyDown(KEY_S))
                positionY += screenHeight * 0.01f;
            positionY = (int)Clamp(positionY, 25.0f, (float)screenHeight - height + 25);
        }
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
    int ballSpeedX;
    int ballSpeedY;
    Color color;
    Texture2D skin;

public:
    ball(int x, int y, int r, Color c, int speedX, int speedY, string name)
    {
        positionX = x;
        positionY = y;
        radius = r;
        color = c;
        ballSpeedX = speedX;
        ballSpeedY = speedY;

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
    void drawBall()
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
    int getBallSpeedX()
    {
        return ballSpeedX;
    }
    int getBallSpeedY()
    {
        return ballSpeedY;
    }
    int getRadius()
    {
        return radius;
    }
    void update(Rectangle leftRec, Rectangle rightRec, int *score1, int *score2)
    {
        if (positionX + radius >= screenWidth || positionX - radius <= 0)
        {
            ballSpeedX *= -1;
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
            ballSpeedY *= -1;
        }
        if (CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
            CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, rightRec))
        {
            ballSpeedX *= -1;
        }
        positionX += ballSpeedX;
        positionY += ballSpeedY;
    }
    ~ball()
    {
        UnloadTexture(skin);
    }
};

struct state
{
    float x;
    float y;
    float p1;
    float p2;
};

int main(void)
{
    int choice;
    cout << "enter your choice: \n";
    cout << "1 for underWater\n2 for fire and ice\n3 for forest and wood\n";
    cout << "4 for neon\n5 for futuristic\n6 for space galaxy\n\n";
    cin >> choice;
    string mode2;
    switch (choice)
    {
    case 1:
        mode2 = "underWater/";
        break;
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
    default:
        mode2 = "underWater/";
        break;
    }
    int oldSW = 600, oldSH = 600;
    int score1 = 0;
    int score2 = 0;

    InitWindow(screenWidth, screenHeight + 30, "Server - Multiplayer Pong");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight};
    theme classic(RED, BLACK, YELLOW, border, 5, mode + mode2 + "background.png");
    paddle left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    paddle right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    scoreBoard score;

    ball gameBall(oldSW / 2, oldSH / 2, (screenWidth * 0.02f), classic.getBallColor(), (screenWidth * 0.007f), (screenHeight * 0.005f), mode + mode2 + "ball.png");
    state sts;
    state dummy = {0, 0, 0, 0};

    ENetHost *host = NULL;
    ENetPeer *peer = NULL;
    networkInitialize(MODE_SERVER, NULL, &host, &peer);
    sts.x = gameBall.getPositionX();
    sts.y = gameBall.getPositionY();
    sts.p1 = left.getPositionY();
    sts.p2 = right.getPositionY();

    Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
    Color buttonColor = WHITE;

    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            int oldSW = screenWidth;
            int oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
            button = {(float)screenWidth - 70, 5, 60, 15};
            int signSpeedX = (gameBall.getBallSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameBall.getBallSpeedY() < 0) ? -1 : 1;

            float newBallX = sts.x * ((float)screenWidth / oldSW);
            float newBallY = sts.y * ((float)screenHeight / oldSH);
            int newBallRadius = (int)(screenWidth * 0.02f);

            // Clamp ball position within the new screen bounds.
            if (gameBall.getPositionX() < newBallRadius)
                gameBall.setPositionX(newBallRadius);
            if (gameBall.getPositionX() > screenWidth - newBallRadius)
                gameBall.setPositionX(screenWidth - newBallRadius);
            if (gameBall.getPositionY() < newBallRadius)
                gameBall.setPositionY(newBallRadius);
            if (gameBall.getPositionY() > screenHeight - 25 - newBallRadius)
                gameBall.setPositionY(screenHeight - 25 - newBallRadius);

            // Update the border and re-create the theme with the new board dimensions.
            border = {0, 25, (float)screenWidth, (float)screenHeight};

            classic.~theme();
            gameBall.~ball();
            left.~paddle();
            right.~paddle();
            new (&left) paddle(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                               (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");

            new (&right) paddle(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");

            new (&gameBall) ball((int)newBallX, (int)newBallY, newBallRadius, classic.getBallColor(),
                                 (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + mode2 + "ball.png");

            new (&classic) theme(RED, BLACK, YELLOW, border, 5, mode + mode2 + "background.png");
        }
        time_t now = time(0);
        struct tm *localTime = localtime(&now);
        char buffer[10];
        strftime(buffer, sizeof(buffer), "%H:%M", localTime);

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPause(isHover, &buttonColor, host);
        if (!isPaused)
        {
            // Update current state (used for networking position scaling).
            sts.x = gameBall.getPositionX();
            sts.y = gameBall.getPositionY();
            sts.p1 = left.getPositionY();
            sts.p2 = right.getPositionY();

            networkProcessEvents(host);
            networkReceiveState(host, &dummy.x, &dummy.y, &dummy.p1, &dummy.p2);
            cout << "Reaceived p2 as" << dummy.p2 << endl;
            dummy.p2 *= screenHeight;
            if (!((std::isinf(dummy.p2) || std::isnan(dummy.p2))) && dummy.p2 >= 0 && dummy.p2 <= screenHeight)
                right.setPositionY((int)dummy.p2);

            gameBall.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());
            left.update();
            right.update();

            networkSendState(host, NULL, (float)gameBall.getPositionX() / screenWidth,
                             (float)gameBall.getPositionY() / screenHeight, (float)left.getPositionY() / screenHeight, (float)right.getPositionY());

            BeginDrawing();
            DrawRectangle(0, 0, screenWidth, 25, BLACK);
            DrawText(TextFormat("Current Time: %s", buffer), 10, 5, 20, WHITE);
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(choice);
            left.drawPaddle();
            right.drawPaddle();
            gameBall.drawBall();
            DrawRectangleRec(button, buttonColor);
            DrawText("Pause", screenWidth - 65, 5, 15, WHITE);
            EndDrawing();
        }
        else
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(button, buttonColor);
            DrawText("Resume", screenWidth - 65, 5, 13, WHITE);
            DrawText("PAUSED!!", screenWidth / 2, screenHeight / 2, 20, WHITE);
            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}
