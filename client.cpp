#include "raylib.h"
#include "network.h"
#include <stdio.h>
#include <ctime>

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
bool isPaused = false; // Global pause state

struct state {
    float x;
    float y;
    float p2;    
    float p1; 
};

float Clamp(float value, float min, float max) {
    return (value < min) ? min : (value > max) ? max : value;
}

bool checkPause(bool isHover, Color* buttonColor) {
    if (isHover) {
        *buttonColor = DARKGRAY;
    } else {
        *buttonColor = WHITE;
    }
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isPaused = !isPaused;
    }
    return isPaused;
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
        DrawRectangle(0, 0, screenWidth, 25, BLACK); // Top bar
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawCircleLines(boundaries.x + boundaries.width/2, boundaries.y + boundaries.height/2, 70, WHITE);
        DrawLine(screenWidth/2, boundaries.y, screenWidth/2, boundaries.y + boundaries.height, WHITE);
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
        if(IsKeyDown(KEY_W)) positionY -= 5;
        if(IsKeyDown(KEY_S)) positionY += 5;
        positionY = Clamp(positionY, 25.0f, 25.0f + screenHeight - height); // Adjusted clamping
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
    int getBallSpeedX() {
        return ballSpeedX;
    }
    int getBallSpeedY() {
        return ballSpeedY;
    }
    int getPositionX() {
        return positionX;
    }
    int getPositionY() {
        return positionY;
    }
    void update(Rectangle leftRec, Rectangle rightRec,int* score1,int* score2) {
        if (positionX + radius >= screenWidth || positionX - radius <= 0) {
            ballSpeedX*=-1;
            if(positionX+radius>=screenWidth) {
                (*score2)++;
            }
            else {
                (*score1)++;
            }
        }
        if (positionY + radius >= screenHeight+25 || positionY - radius <= 25){
            ballSpeedY *= -1;
        } 
        if (CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, leftRec) ||
            CheckCollisionCircleRec((Vector2){(float)positionX, (float)positionY}, radius, rightRec)) {
            ballSpeedX *= -1;
        }
        positionX += ballSpeedX;
        positionY += ballSpeedY;
    }
};

int main(void) {
    int oldSW = screenWidth, oldSH = screenHeight;
    state ballState = {(float)screenWidth/2, (float)screenHeight/2, (float)screenHeight/2, (float)screenHeight/2};

    Color background = {50, 168, 82, 255};
    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight}; // Play area starts at Y=25
    theme classic(RED, background, YELLOW, border, 5);

    paddle left(classic.getBorderWidth()+5, screenHeight/2 - (int)(screenHeight * 0.165f / 2), WHITE,
                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));
    paddle right(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight/2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));

    ball gameBall((int)(ballState.x * ((float)screenWidth / oldSW)), (int)(ballState.y * ((float)screenHeight / oldSH)), 
                  (int)(screenWidth * 0.02f), classic.getBallColor(),
                  (int)(screenWidth * 0.007f), (int)(screenHeight * 0.005f));

    ENetHost* host = NULL;
    ENetPeer* peer = NULL;
    if(networkInitialize(MODE_CLIENT, "192.168.45.197", &host, &peer) != 0) {
        printf("Failed to initialize network\n");
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight + 30, "Client - Multiplayer Pong"); // Include top bar
    SetTargetFPS(60);

    Color buttonColor = WHITE;

    while(!WindowShouldClose()) {
        // Handle window resize
        if (IsWindowResized()) {
            oldSW = screenWidth;
            oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30; // Adjust for top bar
            int signSpeedX= (gameBall.getBallSpeedX() < 0)? -1 : 1;
            int signSpeedY= (gameBall.getBallSpeedY() < 0)? -1 : 1;

            left = paddle(classic.getBorderWidth()+5, screenHeight/2 - (int)(screenHeight * 0.165f / 2), WHITE,
                          (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));
            right = paddle(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight/2 - (int)(screenHeight * 0.165f / 2), WHITE,
                           (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f));

            float newBallX = ballState.x * ((float)screenWidth / oldSW);
            float newBallY = ballState.y * ((float)screenHeight / oldSH);
            int newBallRadius = (int)(screenWidth * 0.02f);
            gameBall = ball((int)newBallX, (int)newBallY, newBallRadius, classic.getBallColor(),
                            (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f));

            border = {0, 25, (float)screenWidth, (float)screenHeight};
            classic = theme(RED, background, YELLOW, border, 5);
        }

        // Pause button interaction
        Vector2 mousePos = GetMousePosition();
        Rectangle button = { (float)screenWidth - 70, 5, 60, 15 };
        bool isHover = CheckCollisionPointRec(mousePos, button);
        isPaused = checkPause(isHover, &buttonColor);

        if (!isPaused) {
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

                // Draw top bar elements
                time_t now = time(0);
                struct tm *localTime = localtime(&now);
                char buffer[10];
                strftime(buffer, sizeof(buffer), "%H:%M", localTime);
                DrawText(TextFormat("Current Time: %s", buffer), 10, 5, 20, WHITE);
                DrawRectangleRec(button, buttonColor);
                DrawText("Pause", screenWidth - 65, 5, 15, WHITE);
            EndDrawing();
        } else {
            BeginDrawing();
                ClearBackground(BLACK);
                DrawRectangleRec(button, buttonColor);
                DrawText("PAUSED!!", screenWidth/2, (screenHeight + 30)/2, 20, WHITE); // Center in window
                DrawText("Resume", screenWidth - 65, 5, 13, WHITE);
            EndDrawing();
        }
    }

    networkShutdown(host);
    CloseWindow();
    return 0;
}