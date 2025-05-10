#include "raylib.h"
#include "network.h"
#include <stdio.h>
#include <ctime>
#include <cmath>
#include <string>
#include <iostream>
using namespace std;

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


int screenHeight = 570, screenWidth = 600; // Adjusted for top bar
bool isPaused = false;                     // Global pause state
string mode = "Assets/";
struct state
{
    float x;
    float y;
    float p2;
    float p1;
};

float Clamp(float value, float min, float max)
{
    return (value < min) ? min : (value > max) ? max
                                               : value;
}

bool checkPauseC(bool isHover, Color *buttonColor, ENetPeer *peer, ENetHost *host)
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
        sendPause(host, peer, isPaused);
    }
    return isPaused;
}

class scoreBoardC
{
    int scoreLeft;
    int scoreRight;

public:
    scoreBoardC()
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

class themeC
{
    Color ballCColor;
    Texture2D pi;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;
    // TODO: add scoreBoardC to client and make it consistent in themeCs
public:
    themeC(Color b, Color ba, Color bo, Rectangle bou, int bw, string name)
    {
        ballCColor = b;
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
        DrawRectangle(0, 0, screenWidth, 25, BLACK); // Top bar
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawTexture(pi, boundaries.x, boundaries.y, WHITE);
        DrawCircleLines(boundaries.x + boundaries.width / 2, boundaries.y + boundaries.height / 2, 70, WHITE);
        DrawLine(screenWidth / 2, boundaries.y, screenWidth / 2, boundaries.y + boundaries.height, WHITE);
    }
    int getBorderWidth()
    {
        return borderWidth;
    }
    Color getballCColor()
    {
        return ballCColor;
    }
    ~themeC()
    {
        UnloadTexture(pi);
    }
};

class paddleC
{
    int height;
    int width;
    int positionX;
    int positionY;
    Texture2D skin;
    Color color;

public:
    paddleC(int x, int y, Color c, int h, int w, string name)
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
    void drawpaddleC()
    {
        DrawTexture(skin, positionX, positionY, WHITE);
    }
    void update()
    {
        if (IsKeyDown(KEY_W))
            positionY -= 5;
        if (IsKeyDown(KEY_S))
            positionY += 5;
        positionY = Clamp(positionY, 25.0f, 25.0f + screenHeight - height); // Adjusted clamping
    }
    Rectangle getRec()
    {
        Rectangle r = {(float)positionX, (float)positionY, (float)width, (float)height};
        return r;
    }
    ~paddleC()
    {
        UnloadTexture(skin);
    }
};

class ballC
{
    int positionX;
    int positionY;
    int radius;
    int ballCSpeedX;
    int ballCSpeedY;
    Color color;
    Texture2D skin;

public:
    ballC(int x, int y, int r, Color c, int speedX, int speedY, string name)
    {
        positionX = x;
        positionY = y;
        radius = r;
        color = c;
        ballCSpeedX = speedX;
        ballCSpeedY = speedY;

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
    void drawballC()
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
    int getballCSpeedX()
    {
        return ballCSpeedX;
    }
    int getballCSpeedY()
    {
        return ballCSpeedY;
    }
    int getPositionX()
    {
        return positionX;
    }
    int getPositionY()
    {
        return positionY;
    }
    void update(Rectangle leftRec, Rectangle rightRec, int *score1, int *score2)
    {
        if (positionX + radius >= screenWidth || positionX - radius <= 0)
        {
            ballCSpeedX *= -1;
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
            ballCSpeedY *= -1;
        }
        if (CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
            CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, rightRec))
        {
            ballCSpeedX *= -1;
        }
        positionX += ballCSpeedX;
        positionY += ballCSpeedY;
    }
    ~ballC()
    {
        UnloadTexture(skin);
    }
};

int runClient()
{
    cout << "Enter ip: ";
    string ip;
    cin >> ip;
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
    ENetHost *host = NULL;
    ENetPeer *peer = NULL;
    if (networkInitialize(MODE_CLIENT, ip.c_str(), &host, &peer) != 0)
    {
        printf("Failed to initialize network\n");
        return 1;
    }
    scoreBoardC score;
    int oldSW = screenWidth, oldSH = screenHeight;
    state ballCState = {(float)screenWidth / 2, (float)screenHeight / 2, (float)screenHeight / 2, (float)screenHeight / 2};
    int signX = 1;
    int signY = 1;
    const double DELAY = 0.1;
    double now = GetTime();
    InitWindow(screenWidth, screenHeight + 30, "Client - Multiplayer Pong"); // Include top bar
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Color background = {50, 168, 82, 255};
    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight}; // Play area starts at Y=25
    themeC classic(RED, background, YELLOW, border, 5, mode + mode2 + "background.png");

    paddleC left(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");
    paddleC right(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");

    ballC gameballC((ballCState.x * ((float)screenWidth / oldSW)), (ballCState.y * ((float)screenHeight / oldSH)),
                  (screenWidth * 0.02f), classic.getballCColor(),
                  (screenWidth * 0.007f), (screenHeight * 0.005f), mode + mode2 + "ball.png");

    Color buttonColor = WHITE;

    while (!WindowShouldClose())
    {
        now = GetTime();
        // Handle window resize
        if (IsWindowResized())
        {
            oldSW = screenWidth;
            oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30; // Adjust for top bar
            int signSpeedX = (gameballC.getballCSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameballC.getballCSpeedY() < 0) ? -1 : 1;

            float newballCX = ballCState.x * ((float)screenWidth / oldSW);
            float newballCY = ballCState.y * ((float)screenHeight / oldSH);
            int newballCRadius = (int)(screenWidth * 0.02f);

            border = {0, 25, (float)screenWidth, (float)screenHeight};
            classic.~themeC();
            gameballC.~ballC();
            left.~paddleC();
            right.~paddleC();
            new (&left) paddleC(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                               (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");

            new (&right) paddleC(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + mode2 + "paddle.png");

            new (&gameballC) ballC((int)newballCX, (int)newballCY, newballCRadius, classic.getballCColor(),
                                 (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + mode2 + "ball.png");

            new (&classic) themeC(RED, background, YELLOW, border, 5, mode + mode2 + "background.png");
        }

        // Pause button interaction
        Vector2 mousePos = GetMousePosition();
        Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
        bool isHover = CheckCollisionPointRec(mousePos, button);
        isPaused = checkPauseC(isHover, &buttonColor, peer, host);

        if (!isPaused)
        {
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
            score.drawBoard(choice);
            left.drawpaddleC();
            right.drawpaddleC();
            gameballC.drawballC();

            // Draw top bar elements
            time_t now = time(0);
            struct tm *localTime = localtime(&now);
            char buffer[10];
            strftime(buffer, sizeof(buffer), "%H:%M", localTime);
            DrawText(TextFormat("Current Time: %s", buffer), 10, 5, 20, WHITE);
            DrawRectangleRec(button, buttonColor);
            DrawText("Pause", screenWidth - 65, 5, 15, WHITE);
            EndDrawing();
        }
        else
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(button, buttonColor);
            DrawText("PAUSED!!", screenWidth / 2, (screenHeight + 30) / 2, 20, WHITE); // Center in window
            DrawText("Resume", screenWidth - 65, 5, 13, WHITE);
            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}

int main() {
    runClient();
}