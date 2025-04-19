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

#define IS_CLIENT 1

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

struct snapshot
{
    float positionY;
    float ballX;
    float ballY;
    double time;
};

float Clamp(float value, float min, float max)
{
    return (value < min) ? min : (value > max) ? max
                                               : value;
}

bool checkPause(bool isHover, Color *buttonColor, ENetPeer *peer, ENetHost *host)
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

class theme
{
    Color ballColor;
    Texture2D pi;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;
//TODO: add scoreboard to client and make it consistent in themes
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
    void drawPaddle()
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
    int getBallSpeedX()
    {
        return ballSpeedX;
    }
    int getBallSpeedY()
    {
        return ballSpeedY;
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

int main(void)
{
    cout << "Enter ip: ";
    string ip;
    cin >> ip;
    int choice;
    cout<<"enter your choice: \n";
    cout<<"1 for underWater\n2 for fire and ice\n";
    cin>>choice;
    string mode2;
    switch (choice)
    {
    case 1:
        mode2 = "underWater/";
        break;
    case 2:
        mode2 = "fireAndIce/";
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
    cout << ".............................\n............................";
    int oldSW = screenWidth, oldSH = screenHeight;
    state ballState = {(float)screenWidth / 2, (float)screenHeight / 2, (float)screenHeight / 2, (float)screenHeight / 2};
    int signX = 1;
    int signY = 1;
    const double DELAY = 0.1;
    double now = GetTime();
    InitWindow(screenWidth, screenHeight + 30, "Client - Multiplayer Pong"); // Include top bar
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    snapshot snaps[2] = {{float(screenHeight / 2 - (int)(screenHeight * 0.165f / 2)),
                          (ballState.x * ((float)screenWidth / oldSW)),
                          (ballState.y * ((float)screenHeight / oldSH)),
                          now},
                         {float(screenHeight / 2 - (int)(screenHeight * 0.165f / 2)),
                          (ballState.x * ((float)screenWidth / oldSW)),
                          (ballState.y * ((float)screenHeight / oldSH)),
                          now}};

    Color background = {50, 168, 82, 255};
    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight}; // Play area starts at Y=25
    theme classic(RED, background, YELLOW, border, 5, mode+mode2+"background.png");

    paddle left(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode+mode2+"paddle.png");
    paddle right(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode+mode2+"paddle.png");

    ball gameBall((ballState.x * ((float)screenWidth / oldSW)), (ballState.y * ((float)screenHeight / oldSH)),
                  (screenWidth * 0.02f), classic.getBallColor(),
                  (screenWidth * 0.007f), (screenHeight * 0.005f), mode+mode2+"ball.png");

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
            int signSpeedX = (gameBall.getBallSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameBall.getBallSpeedY() < 0) ? -1 : 1;

            float newBallX = ballState.x * ((float)screenWidth / oldSW);
            float newBallY = ballState.y * ((float)screenHeight / oldSH);
            int newBallRadius = (int)(screenWidth * 0.02f);

            border = {0, 25, (float)screenWidth, (float)screenHeight};
            classic.~theme();
            gameBall.~ball();
            left.~paddle();
            right.~paddle();
            new (&left) paddle(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                               (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode+mode2+"paddle.png");

            new (&right) paddle(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode+mode2+"paddle.png");

            new (&gameBall) ball((int)newBallX, (int)newBallY, newBallRadius, classic.getBallColor(),
                                 (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode+mode2+"ball.png");

            new (&classic) theme(RED, background, YELLOW, border, 5, mode+mode2+"background.png");
        }

        // Pause button interaction
        Vector2 mousePos = GetMousePosition();
        Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
        bool isHover = CheckCollisionPointRec(mousePos, button);
        isPaused = checkPause(isHover, &buttonColor, peer, host);

        if (!isPaused)
        {
            networkSendState(host, peer, 0, 0, 0, (float)right.getPositionY() / screenHeight);
            cout << "Sent as: " << (float)right.getPositionY() / screenHeight << endl;
            float dummy;
            networkReceiveState(host, &ballState.x, &ballState.y, &ballState.p1, &dummy);
            cout << "Received: " << ballState.x << " " << ballState.y << " " << ballState.p1 << "\n";
            ballState.x *= screenWidth;
            ballState.y *= screenHeight;
            ballState.p1 *= screenHeight;
            if (!(std::isinf(ballState.p1) || std::isnan(ballState.p1)) && !(std::isinf(ballState.x) || std::isnan(ballState.x)) && !(std::isinf(ballState.y) || std::isnan(ballState.y)) && ballState.x > 0 && ballState.x < screenWidth && ballState.y > 0 && ballState.y < screenHeight && ballState.p1 >= 0 && ballState.p1 < screenHeight)
            {

                snaps[0] = snaps[1];
                snaps[1] = {ballState.p1, ballState.x, ballState.y, now};
                double renderTime = now - DELAY;
                double dt = snaps[1].time - snaps[0].time;
                float drawY = snaps[1].positionY;
                float drawBallX = snaps[1].ballX;
                float drawBallY = snaps[1].ballY;

                if (dt > 0)
                {
                    float t = float((renderTime - snaps[0].time) / dt);
                    if (t < 0)
                        t = 0;
                    else if (t > 1)
                        t = 1;

                    drawY = snaps[0].positionY + (snaps[1].positionY - snaps[0].positionY) * t;
                    float drawBallX = snaps[0].ballX + (snaps[1].ballX - snaps[0].ballX) * t;
                    float drawBallY = snaps[0].ballY + (snaps[1].ballY - snaps[0].ballY) * t;
                }
                left.setPositionY(ballState.p1);
                gameBall.setPositionX(ballState.x);
                gameBall.setPositionY(ballState.y);
            }

            right.update();
            BeginDrawing();
            classic.drawBoard();
            left.drawPaddle();
            right.drawPaddle();
            gameBall.drawBall();

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