#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <cmath>
#include <iostream>
using namespace std;

#define DrawText DrawText  
#define CloseWindow CloseWindow
#define ShowCursor ShowCursor

int screenWidth = 700;
int screenHeight = 570;
bool isPaused = false;
string mode = "Assets/";
int gameMode = 1; // 1 for Computer, 2 for 2 Players

float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

bool checkPause(bool isHover, Color *buttonColor) {
    if (isHover) *buttonColor = DARKGRAY;
    else *buttonColor = WHITE;
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
        isPaused = !isPaused;
    if (IsKeyPressed(KEY_P)) 
        isPaused = !isPaused;
    return isPaused;
}

class scoreBoard {
    int scoreLeft = 0;
    int scoreRight = 0;
public:
    void drawBoard(int choice) {
        float posX, posX2, posY, fontSize;
        Color scoreColor;
        switch(choice) {
            case 1: posX = 0.175*screenWidth; posX2=0.6875*screenWidth; posY=0.375*screenHeight;
                    scoreColor = (Color){120,252,255,90}; fontSize=0.1625*(screenWidth+screenHeight); break;
            case 2: posX=0.1125*screenWidth; posX2=0.82125*screenWidth; posY=0.4575*screenHeight;
                    scoreColor=(Color){150,255,255,255}; fontSize=0.075*(screenWidth+screenHeight); break;
            case 3: posX=0.1125*screenWidth; posX2=0.82125*screenWidth; posY=0.4575*screenHeight;
                    scoreColor=(Color){124,252,0,255}; fontSize=0.075*(screenWidth+screenHeight); break;
            default: posX=0.175*screenWidth; posX2=0.6875*screenWidth; posY=0.375*screenHeight;
                     scoreColor=(Color){120,252,255,90}; fontSize=0.1625*(screenWidth+screenHeight); break;
        }
        DrawText(TextFormat("%d", scoreLeft), posX2, posY, fontSize, scoreColor);
        DrawText(TextFormat("%d", scoreRight), posX, posY, fontSize, scoreColor);
    }
    int* getScore1() { return &scoreLeft; }
    int* getScore2() { return &scoreRight; }
};

class theme {
    Color ballSColor;
    Texture2D pic;
    Color background;
    Rectangle boundaries;
    int borderWidth;
public:
    theme(Color b, Color ba, Rectangle bou, int bw, string name) : 
        ballSColor(b), background(ba), boundaries(bou), borderWidth(bw) {
        Image bck = LoadImage(name.c_str());
        ImageResize(&bck, bou.width, bou.height);
        pic = LoadTextureFromImage(bck);
        UnloadImage(bck);
    }
    void drawBoard(int, int) {
        ClearBackground(background);
        DrawRectangle(0, 0, screenWidth, 25, BLACK);
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawTexture(pic, boundaries.x, boundaries.y, WHITE);
        DrawCircleLines(boundaries.x + boundaries.width/2, boundaries.y + boundaries.height/2, 70, WHITE);
        DrawLine(screenWidth/2, boundaries.y, screenWidth/2, boundaries.y + boundaries.height, WHITE);
    }
    Color getballSColor() { return ballSColor; }
    ~theme() { UnloadTexture(pic); }
};

class paddle {
    int height, width, positionX, positionY;
    Texture2D skin;
public:
    paddle(int x, int y, int h, int w, string name) : 
        positionX(x), positionY(y), height(h), width(w) {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, width, height);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    void setPositionY(int y) { positionY = y; }
    int getPositionY() { return positionY; }
    int getHeight() { return height; } // Added getHeight method
    void drawpaddleS() { DrawTexture(skin, positionX, positionY, WHITE); }
    void update(bool isLeft) {
        if(isLeft) {
            if(IsKeyDown(KEY_W)) positionY -= screenHeight*0.01f;
            if(IsKeyDown(KEY_S)) positionY += screenHeight*0.01f;
        } else {
            if(IsKeyDown(KEY_UP)) positionY -= screenHeight*0.01f;
            if(IsKeyDown(KEY_DOWN)) positionY += screenHeight*0.01f;
        }
        positionY = (int)Clamp(positionY, 25.0f, (float)screenHeight-height+25);
    }
    Rectangle getRec() { return {(float)positionX, (float)positionY, (float)width, (float)height}; }
    ~paddle() { UnloadTexture(skin); }
};

class ball {
    int positionX, positionY, radius, ballSSpeedX, ballSSpeedY;
    Texture2D skin;
public:
    ball(int x, int y, int r, int speedX, int speedY, string name) : 
        positionX(x), positionY(y), radius(r), ballSSpeedX(speedX), ballSSpeedY(speedY) {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, radius*2, radius*2);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    void drawballS() { DrawTexture(skin, positionX-radius, positionY-radius, WHITE); }
    void update(Rectangle leftRec, Rectangle rightRec, int* score1, int* score2) {
        if(positionX+radius >= screenWidth || positionX-radius <= 0) {
            ballSSpeedX *= -1;
            (positionX+radius >= screenWidth) ? (*score2)++ : (*score1)++;
        }
        if(positionY+radius >= screenHeight+25 || positionY-radius <= 25)
            ballSSpeedY *= -1;
        if(CheckCollisionCircleRec({(float)positionX, (float)positionY}, radius, leftRec) || 
           CheckCollisionCircleRec({(float)positionX, (float)positionY}, radius, rightRec))
            ballSSpeedX *= -1;
        positionX += ballSSpeedX;
        positionY += ballSSpeedY;
    }
    int getPositionX() { return positionX; }
    int getPositionY() { return positionY; }
    ~ball() { UnloadTexture(skin); }
};

void winningPage(int winner) {
    const char *path = NULL;
    switch (winner) {
        case 0: 
            if(gameMode == 1){ 
                path = "Assets/winningPage/playerAi.jpg";
            }
            else{ 
                path = "Assets/winningPage/player1.jpg";
            }
            break;
        case 1: path = "Assets/winningPage/player2.jpg"; break;
        default: path = "Assets/winningPage/tie.jpg"; break;
    }
    Texture2D image = LoadTexture(path);
    Rectangle dest = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawTexturePro(image, (Rectangle){0,0,(float)image.width,(float)image.height}, 
                       dest, (Vector2){0,0}, 0.0f, WHITE);
        EndDrawing();
    }
    UnloadTexture(image);
}

int main() {
    int choice;
    int frameNo;
    cout << "Choose theme:\n1: Underwater\n2: Fire/Ice\n3: Forest/Wood\n4: Neon\n5: Futuristic\n6: Space\n";
    cin >> choice;
    cout << "Choose game mode:\n1: Computer\n2: 2 Players\n";
    cin >> gameMode;

    string mode2;
    const char* themes[] = {"underWater/", "fireAndIce/", "forestAndWood/", "neon/", "futuristic/", "spacegalaxy/"};
    mode2 = themes[choice-1];

    InitWindow(screenWidth, screenHeight+30, "Offline Pong");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Rectangle border = {0,25,(float)screenWidth,(float)screenHeight};
    theme classic(RED, BLACK, border, 5, mode + mode2 + "background.png");
    paddle left(10, screenHeight/2-(int)(screenHeight*0.165f/2), 
            (int)(screenHeight*0.165f), (int)(screenWidth*0.02f), mode + mode2 + "paddle.png");
    paddle right(screenWidth-30, screenHeight/2-(int)(screenHeight*0.165f/2), 
                 (int)(screenHeight*0.165f), (int)(screenWidth*0.02f), mode + mode2 + "paddle.png");
    scoreBoard score;

    ball gameballS(screenWidth/2, screenHeight/2, (screenWidth*0.02f), 
                   (screenWidth*0.007f), (screenHeight*0.005f), mode + mode2 + "ball.png");
    Rectangle button = {(float)screenWidth-70,5,60,15};
    Color buttonColor = WHITE;
    Image paused = LoadImage("Assets/winningPage/paused.jpg");
    Texture2D pausedTexture = LoadTextureFromImage(paused);

    while(!WindowShouldClose()) {
        if(IsWindowResized()) {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight()-30;
            button = {(float)screenWidth-70,5,60,15};
            border = {0,25,(float)screenWidth,(float)screenHeight};
            classic.~theme();
            new (&classic) theme(RED, BLACK, border, 5, mode + mode2 + "background.png");
        }

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPause(isHover, &buttonColor);

        if(!isPaused) {
            left.update(true);

            if (gameMode == 1) {
                int ballY = gameballS.getPositionY();
                int paddleHeight = right.getHeight();
                int targetY = ballY - paddleHeight / 2;
                int currentY = right.getPositionY();
                float moveStep = screenHeight * 0.01f;

                if (currentY < targetY) {
                    currentY += moveStep;
                    if (currentY > targetY) currentY = targetY;
                } else if (currentY > targetY) {
                    currentY -= moveStep;
                    if (currentY < targetY) currentY = targetY;
                }

                currentY = Clamp(currentY, 25.0f, screenHeight - paddleHeight +25);
                right.setPositionY(currentY);
            } else {
                right.update(false); 
            }

            gameballS.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());

            BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(choice);
            left.drawpaddleS();
            right.drawpaddleS();
            gameballS.drawballS();
            DrawText(TextFormat("%d", (int)frameNo/60), 10, 5, 20, WHITE);
            DrawRectangleRec(button, buttonColor);
            DrawText("Pause", screenWidth-65,5,15,WHITE);
            EndDrawing();
        } else {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(button, buttonColor);
            DrawTexturePro(pausedTexture, (Rectangle){0,0,(float)pausedTexture.width,(float)pausedTexture.height},
                           (Rectangle){0,0,(float)GetScreenWidth(),(float)GetScreenHeight()-30}, (Vector2){0,0}, 0.0f, WHITE);
            EndDrawing();
        }
        frameNo++;
        if((int)frameNo / 60 >=59) break;
    }

    if(*(score.getScore1()) > *(score.getScore2())) winningPage(0);
    else if(*(score.getScore1()) < *(score.getScore2())) winningPage(1);
    else winningPage(2);

    CloseWindow();
    return 0;
}