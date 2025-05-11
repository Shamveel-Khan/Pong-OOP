#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <cmath>
#include <iostream>
using namespace std;

#define DrawText DrawText  // Fix conflicting macro definitions
#define CloseWindow CloseWindow
#define ShowCursor ShowCursor

int screenWidth = 700;
int screenHeight = 570;
bool isPaused = false;
string mode = "Assets/";

float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

bool checkPauseOffline(bool isHover, Color *buttonColor) {
    if (isHover) *buttonColor = DARKGRAY;
    else *buttonColor = WHITE;
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        isPaused = !isPaused;
    return isPaused;
}

class scoreBoardOffline {
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
        DrawText(TextFormat("%d", scoreRight), posX2, posY, fontSize, scoreColor);
        DrawText(TextFormat("%d", scoreLeft), posX, posY, fontSize, scoreColor);
    }
    int* getScore1() { return &scoreLeft; }
    int* getScore2() { return &scoreRight; }
};

class themeOffline {
    Color ballSColor;
    Texture2D pic;
    Color background;
    Rectangle boundaries;
    int borderWidth;
public:
    themeOffline(Color b, Color ba, Rectangle bou, int bw, string name) : 
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
    ~themeOffline() { UnloadTexture(pic); }
};

class paddleOffline {
    int height, width, positionX, positionY;
    Texture2D skin;
public:
    paddleOffline(int x, int y, int h, int w, string name) : 
        positionX(x), positionY(y), height(h), width(w) {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, width, height);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    void setPositionY(int y) { positionY = y; }
    int getPositionY() { return positionY; }
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
    ~paddleOffline() { UnloadTexture(skin); }
};

class ballOffline {
    int positionX, positionY, radius, ballSSpeedX, ballSSpeedY;
    Texture2D skin;
public:
    ballOffline(int x, int y, int r, int speedX, int speedY, string name) : 
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
    ~ballOffline() { UnloadTexture(skin); }
};

int main() {
    int choice;
    cout << "Choose theme:\n1: Underwater\n2: Fire/Ice\n3: Forest/Wood\n4: Neon\n5: Futuristic\n6: Space\n";
    cin >> choice;
    string mode2;
    const char* themes[] = {"underWater/", "fireAndIce/", "forestAndWood/", "neon/", "futuristic/", "spacegalaxy/"};
    mode2 = themes[choice-1];

    InitWindow(screenWidth, screenHeight+30, "Offline Pong");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Rectangle border = {0,25,(float)screenWidth,(float)screenHeight};
    themeOffline classic(RED, BLACK, border, 5, mode + mode2 + "background.png");
    paddleOffline left(10, screenHeight/2-(int)(screenHeight*0.165f/2), 
            (int)(screenHeight*0.165f), (int)(screenWidth*0.02f), mode + mode2 + "paddle.png");
    paddleOffline right(screenWidth-30, screenHeight/2-(int)(screenHeight*0.165f/2), 
                 (int)(screenHeight*0.165f), (int)(screenWidth*0.02f), mode + mode2 + "paddle.png");
    scoreBoardOffline score;

    ballOffline gameballS(screenWidth/2, screenHeight/2, (screenWidth*0.02f), 
                   (screenWidth*0.007f), (screenHeight*0.005f), mode + mode2 + "ball.png");
    Rectangle button = {(float)screenWidth-70,5,60,15};
    Color buttonColor = WHITE;

    while(!WindowShouldClose()) {
        if(IsWindowResized()) {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight()-30;
            button = {(float)screenWidth-70,5,60,15};
            border = {0,25,(float)screenWidth,(float)screenHeight};
            classic.~themeOffline();
            new (&classic) themeOffline(RED, BLACK, border, 5, mode + mode2 + "background.png");
        }

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPauseS(isHover, &buttonColor);

        if(!isPaused) {
            left.update(true);
            right.update(false);
            gameballS.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());

            BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(choice);
            left.drawpaddleS();
            right.drawpaddleS();
            gameballS.drawballS();
            DrawRectangleRec(button, buttonColor);
            DrawText("Pause", screenWidth-65,5,15,WHITE);
            EndDrawing();
        } else {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(button, buttonColor);
            DrawText("Resume", screenWidth-65,5,13,WHITE);
            DrawText("PAUSED!!", screenWidth/2, screenHeight/2, 20, WHITE);
            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}