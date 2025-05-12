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

int screenWidth = 800;
int screenHeight = 770;
bool isPaused = false;
int isServer = false;
int runOnline = 0;
string mode = "Assets/";

Rectangle ScreenBounds;

typedef struct
{
    int scoreL;
    int scoreR;
} scores;

// below is for pong offline:
float Clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

bool checkPauseOffline(bool isHover, Color *buttonColor)
{
    if (isHover)
        *buttonColor = DARKGRAY;
    else
        *buttonColor = WHITE;
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        isPaused = !isPaused;
    return isPaused;
}

class scoreBoardOffline
{
    int scoreLeft = 0;
    int scoreRight = 0;

public:
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
            scoreColor = (Color){255,172,28,255};
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
    int *getScore1() { return &scoreLeft; }
    int *getScore2() { return &scoreRight; }
};

class themeOffline
{
    Color ballSColor;
    Texture2D pic;
    Color background;
    Rectangle boundaries;
    int borderWidth;

public:
    themeOffline(Color b, Color ba, Rectangle bou, int bw, string name) : ballSColor(b), background(ba), boundaries(bou), borderWidth(bw)
    {
        Image bck = LoadImage(name.c_str());
        ImageResize(&bck, bou.width, bou.height);
        pic = LoadTextureFromImage(bck);
        UnloadImage(bck);
    }
    void drawBoard(int, int)
    {
        ClearBackground(background);
        DrawRectangle(0, 0, screenWidth, 25, BLACK);
        DrawRectangleLinesEx(boundaries, borderWidth, YELLOW);
        DrawTexture(pic, boundaries.x, boundaries.y, WHITE);
        DrawCircleLines(boundaries.x + boundaries.width / 2, boundaries.y + boundaries.height / 2, 70, WHITE);
        DrawLine(screenWidth / 2, boundaries.y, screenWidth / 2, boundaries.y + boundaries.height, WHITE);
    }
    Color getballSColor() { return ballSColor; }
    ~themeOffline() { UnloadTexture(pic); }
};

class paddleOffline
{
    int height, width, positionX, positionY;
    Texture2D skin;

public:
    paddleOffline(int x, int y, int h, int w, string name) : positionX(x), positionY(y), height(h), width(w)
    {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, width, height);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    void setPositionY(int y) { positionY = y; }
    int getPositionY() { return positionY; }
    void drawpaddleS() { DrawTexture(skin, positionX, positionY, WHITE); }
    void update(bool isLeft)
    {
        if (isLeft)
        {
            if (IsKeyDown(KEY_W))
                positionY -= screenHeight * 0.01f;
            if (IsKeyDown(KEY_S))
                positionY += screenHeight * 0.01f;
        }
        else
        {
            if (IsKeyDown(KEY_UP))
                positionY -= screenHeight * 0.01f;
            if (IsKeyDown(KEY_DOWN))
                positionY += screenHeight * 0.01f;
        }
        positionY = (int)Clamp(positionY, 25.0f, (float)screenHeight - height + 25);
    }
    Rectangle getRec() { return {(float)positionX, (float)positionY, (float)width, (float)height}; }
    ~paddleOffline() { UnloadTexture(skin); }
};

class ballOffline
{
    int positionX, positionY, radius, ballSSpeedX, ballSSpeedY;
    Texture2D skin;

public:
    ballOffline(int x, int y, int r, int speedX, int speedY, string name) : positionX(x), positionY(y), radius(r), ballSSpeedX(speedX), ballSSpeedY(speedY)
    {
        Image skinImg = LoadImage(name.c_str());
        ImageResize(&skinImg, radius * 2, radius * 2);
        skin = LoadTextureFromImage(skinImg);
        UnloadImage(skinImg);
    }
    int getBallSSpeedX() { return ballSSpeedX; }
    int getBallSSpeedY() { return ballSSpeedY; }
    void drawballS() { DrawTexture(skin, positionX - radius, positionY - radius, WHITE); }
    void update(Rectangle leftRec, Rectangle rightRec, int *score1, int *score2)
    {
        if (positionX + radius >= screenWidth || positionX - radius <= 0)
        {
            ballSSpeedX *= -1;
            (positionX + radius >= screenWidth) ? (*score2)++ : (*score1)++;
        }
        if (positionY + radius >= screenHeight + 25 || positionY - radius <= 25)
            ballSSpeedY *= -1;
        if (CheckCollisionCircleRec({(float)positionX, (float)positionY}, radius, leftRec) ||
            CheckCollisionCircleRec({(float)positionX, (float)positionY}, radius, rightRec))
            ballSSpeedX *= -1;
        positionX += ballSSpeedX;
        positionY += ballSSpeedY;
    }
    int getPositionX() { return positionX; }
    int getPositionY() { return positionY; }
    ~ballOffline() { UnloadTexture(skin); }
};

// the below is for client pong

struct state
{
    float x;
    float y;
    float p2;
    float p1;
};

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
        case 4:
            posX = 0.175 * screenWidth;
            posX2 = 0.6875 * screenWidth;
            posY = 0.375 * screenHeight;
            scoreColor = (Color){255,172,28,255};
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

class themeC
{
    Color ballCColor;
    Texture2D pi;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;

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

// below is for server pong
bool checkPauseS(bool isHover, Color *buttonColor, ENetHost *host)
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

class scoreBoardS
{
    int scoreLeft;
    int scoreRight;

public:
    scoreBoardS()
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
            scoreColor = (Color){255,172,28,255};
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

class themeS
{
    Color ballSColor;
    Texture2D pic;
    Color background;
    Color border;
    Rectangle boundaries;
    int borderWidth;

public:
    themeS(Color b, Color ba, Color bo, Rectangle bou, int bw, string name)
    {
        ballSColor = b;
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
    Color getballSColor()
    {
        return ballSColor;
    }
    ~themeS()
    {
        UnloadTexture(pic);
    }
};

class paddleS
{
    int height;
    int width;
    int positionX;
    int positionY;
    Texture2D skin;
    Color color;

public:
    paddleS(int x, int y, Color c, int h, int w, string name)
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
    void drawpaddleS()
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
    ~paddleS()
    {
        UnloadTexture(skin);
    }
};

class ballS
{
    int positionX;
    int positionY;
    int radius;
    int ballSSpeedX;
    int ballSSpeedY;
    Color color;
    Texture2D skin;

public:
    ballS(int x, int y, int r, Color c, int speedX, int speedY, string name)
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
    void drawballS()
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
    int getballSSpeedX()
    {
        return ballSSpeedX;
    }
    int getballSSpeedY()
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
    ~ballS()
    {
        UnloadTexture(skin);
    }
};

const Color TextColor = {132, 132, 132, 255}; // White
const Color HeadingColor = {26, 37, 37, 255};
const Color BACKGROUND_COLOR = {26, 37, 37, 255};   // #1A2525
const Color CONTAINER_COLOR = {233, 216, 166, 255}; // #E9D8A6
const Color BUTTON_ORANGE = {238, 155, 0, 255};
const Color BUTTON_ORANGE_HOVER = {255, 183, 3, 255};
const Color BUTTON_ORANGE_PRESSED = {204, 125, 0, 255};
const Color BUTTON_CERULEAN = {25, 130, 196, 255};
const Color BUTTON_CERULEAN_HOVER = {45, 150, 216, 255};
const Color BUTTON_CERULEAN_PRESSED = {15, 110, 166, 255};
const Color BUTTON_RUST = {187, 62, 3, 255};
const Color BUTTON_RUST_HOVER = {216, 99, 23, 255};
const Color BUTTON_RUST_PRESSED = {148, 49, 0, 255};
const Color BUTTON_CARMINE = {155, 34, 38, 255};
const Color BUTTON_CARMINE_HOVER = {186, 58, 62, 255};
const Color BUTTON_CARMINE_PRESSED = {124, 27, 30, 255};
const Color BUTTON_FOREST = {34, 139, 34, 255};
const Color BUTTON_FOREST_HOVER = {54, 159, 54, 255};
const Color BUTTON_FOREST_PRESSED = {24, 119, 24, 255};
enum GAMESTATES
{
    LOBBY,
    MODE,
    GAMEPLAY, // kala kala
    EXIT,
    THEME,
    ABOUT,
    ONLINEGAME // kala kala2
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
enum UI_TYPES
{
    BUTTON,
    LABEL,
    CONTAINER
};
enum MODES
{
    ONLINE,
    OFFLINE,
    COMPUTER
};

THEMES currentTheme = THEME4;
GAMESTATES currentState = LOBBY;
MODES currentMode = OFFLINE;
// helper function to redirect to links
void openLink(const string link)
{
    system(("start " + link).c_str());
}

class InputBox;
class Container;
class Background;
InputBox *globalIPInput = nullptr;

// UI Base Class with built-in font handling
class UIComponent
{
protected:
    Vector2 originalPosition, originalSize;
    Vector2 position, size;
    Color color;
    UI_TYPES type;

    static bool fontsLoaded;

public:
    static Font primaryFont, secondaryFont;
    static float scaleX, scaleY;

    static void LoadFonts(const char *primaryPath, const char *secondaryPath)
    {
        if (!fontsLoaded)
        {
            // Load fonts with larger base size for better quality
            primaryFont = LoadFontEx(primaryPath, 128, 0, 0);
            secondaryFont = LoadFontEx(secondaryPath, 128, 0, 0);
            SetTextureFilter(primaryFont.texture, TEXTURE_FILTER_BILINEAR);
            SetTextureFilter(secondaryFont.texture, TEXTURE_FILTER_BILINEAR);
            fontsLoaded = true;
        }
    }

    static void UnloadFonts()
    {
        if (fontsLoaded)
        {
            UnloadFont(primaryFont);
            UnloadFont(secondaryFont);
            fontsLoaded = false;
        }
    }

    UIComponent(Vector2 pos, Vector2 sz, Color col)
        : originalPosition(pos), originalSize(sz), color(col)
    {
        updatePosition();
    }
    UIComponent() : originalPosition({0, 0}), originalSize({0, 0}), color(BLANK) {}

    static void setScale(float sx, float sy)
    {
        scaleX = sx;
        scaleY = sy;
    }

    virtual void updatePosition()
    {
        // Pixel-perfect positioning
        position.x = roundf(originalPosition.x * scaleX);
        position.y = roundf(originalPosition.y * scaleY);
        size.x = roundf(originalSize.x * scaleX);
        size.y = roundf(originalSize.y * scaleY);
    }

    virtual void draw() = 0;
    virtual void handleEvent() = 0;
};

float UIComponent::scaleX = 1.0f;
float UIComponent::scaleY = 1.0f;
Font UIComponent::primaryFont = {0};
Font UIComponent::secondaryFont = {0};
bool UIComponent::fontsLoaded = false;

class Ball
{
private:
    float originalY, originalX;
    float originalRadius, originalSpeed;
    float y, x;
    float radius;
    float speed;
    Color color;

public:
    Ball()
    {

        originalY = 100;
        originalX = 50;
        originalRadius = 20;
        originalSpeed = 5;
        updatePosition();
    }

    Ball(float y, float x, float radius, float speed, Color col = WHITE)
    {
        this->originalY = y;
        this->originalRadius = radius;
        this->originalSpeed = speed;
        this->originalX = x;
        color = col;
        updatePosition();
    }

    void Update()
    {

        y += speed;
        if (y - radius <= 0 || y + radius >= ScreenBounds.height)
        {
            speed *= -1;
        }
    }
    void updatePosition()
    {
        y = roundf(originalY * UIComponent::scaleY);
        x = roundf(originalX * UIComponent::scaleX);
        radius = roundf(originalRadius * min(UIComponent::scaleX, UIComponent::scaleY));
        speed = roundf(originalSpeed * min(UIComponent::scaleX, UIComponent::scaleY));
    }

    void Draw() const
    {
        DrawCircle(static_cast<int>(x), static_cast<int>(y), radius, Fade(color, 0.9f));
    }

    friend class Background;
};

// Label Class
class Label : public UIComponent
{
    friend class Container;
    string text;
    int fontSizeOriginal;
    Color textColor;
    bool useSecondaryFont;
    int fontSize;
    Vector2 textPosition;

public:
    Label(Vector2 pos, string txt, int fs, Color col, bool secondary = false)
        : UIComponent(pos, {0, 0}, BLANK),
          text(txt),
          fontSizeOriginal(fs),
          textColor(col),
          useSecondaryFont(secondary),
          fontSize(fs)
    {
        type = LABEL;
        updatePosition();
    }

    Label() : UIComponent(), text(""), fontSizeOriginal(0),
              textColor(BLANK), useSecondaryFont(false), fontSize(0)
    {
        type = LABEL;
    }

    void updatePosition() override
    {
        UIComponent::updatePosition();
        fontSize = fontSizeOriginal * min(scaleX, scaleY);
        Font &font = useSecondaryFont ? secondaryFont : primaryFont;
        Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, 1);
        textPosition = {
            position.x - textSize.x / 2,
            position.y - textSize.y / 2};
    }

    void draw() override
    {
        Font &font = useSecondaryFont ? secondaryFont : primaryFont;
        DrawTextEx(font, text.c_str(), textPosition, fontSize, 1, textColor);
    }

    void handleEvent() override {}
    virtual ~Label() {}
};

// Button Class (constructor updated to 8 params; originalColor defaults to col)
class Button : public UIComponent
{
    friend class Container;

protected:
    string text;
    int fontSizeOriginal, fontSize;
    Color textColor, hoverColor, pressedColor, originalColor;
    Vector2 textPosition;
    bool isHovered = false, isPressed = false;

public:
    Button(Vector2 pos, Vector2 sz, Color col,
           string txt, int fontSize, Color textCol,
           Color hoverCol, Color pressCol)
        : UIComponent(pos, sz, col),
          text(txt),
          fontSizeOriginal(fontSize),
          textColor(textCol),
          hoverColor(hoverCol),
          pressedColor(pressCol),
          originalColor(col)
    {
        type = BUTTON;
        updatePosition();
    }

    Button() : UIComponent(), text(""), fontSizeOriginal(0),
               textColor(BLANK), hoverColor(BLANK),
               pressedColor(BLANK), originalColor(BLANK),
               fontSize(0)
    {
        type = BUTTON;
    }

    void updatePosition() override
    {
        UIComponent::updatePosition();
        float u = min(scaleX, scaleY);
        fontSize = static_cast<int>(round(fontSizeOriginal * u));
        Vector2 textSize = MeasureTextEx(primaryFont, text.c_str(), fontSize, 1);
        textPosition = {
            position.x + (size.x - textSize.x) * 0.5f,
            position.y + (size.y - textSize.y) * 0.5f};
    }

    void draw() override
    {
        // Smoother rounded rectangles (increased segments)
        DrawRectangleRounded({position.x, position.y, size.x, size.y}, 0.3f, 32, color);

        DrawTextEx(primaryFont, text.c_str(),
                   Vector2{roundf(textPosition.x), roundf(textPosition.y)},
                   fontSize, 0, textColor);
    }

    virtual void click()
    {
        cout << "Button clicked: " << text << endl;
    }

    void handleEvent() override
    {
        Vector2 mp = GetMousePosition();
        Rectangle r = {position.x, position.y, size.x, size.y};
        bool prevHovered = isHovered;
        isHovered = CheckCollisionPointRec(mp, r);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isHovered)
        {
            isPressed = true;
            color = pressedColor;
            click();
        }
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            isPressed = false;
            color = isHovered ? hoverColor : originalColor;
        }
        else if (isHovered != prevHovered)
        {
            color = isHovered ? hoverColor : originalColor;
        }
    }
};

// Specialized Buttons
class ButtonStart : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentState = GAMEPLAY;
        cout << "Start Game button clicked!" << endl;
    }
};
class ButtonQuit : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentState = EXIT;
        CloseWindow();
        cout << "Quit Game button clicked!" << endl;
    }
};
class ButtonClose : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = LOBBY;
        cout << "Button Close Clicked!" << endl;
    }
};
class ButtonTheme : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = THEME;
    }
};
class ButtonOnline : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = ONLINEGAME;
        currentMode = ONLINE;
        cout << "ONLINE Button Clicked!" << endl;
    }
};
class ButtonOffline : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = LOBBY;
        currentMode = OFFLINE;
        cout << "OFFLINE Button Clicked!" << endl;
    }
};
class ButtonComputer : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = LOBBY;
        currentMode = COMPUTER;
        cout << "Computer Button Clicked!" << endl;
    }
};
class ButtonMode : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = MODE;
        cout << "MODE BUTTON Clicked!" << endl;
    }
};
class ButtonAbout : public Button
{
    using Button::Button;
    void click() override
    {
        currentState = ABOUT;
        cout << "MODE BUTTON Clicked!" << endl;
    }
};
class ButtonViewProfile : public Button
{
    using Button::Button;

private:
    string link;

public:
    void click() override
    {
        openLink((link));
        cout << "VIEW PROFILE Button Clicked!" << endl;
    }
    void setLink(const string &l) { link = l; } // Setter for link
};
// Input class
class InputBox : public UIComponent
{
    friend class Container;
    string text;
    bool focused = false;
    int cursorBlink = 0;

public:
    InputBox(Vector2 pos, Vector2 sz, Color col)
        : UIComponent(pos, sz, col)
    {
        text.reserve(16); // For IP (xxx.xxx.xxx.xxx)
    }

    void handleEvent() override
    {
        Vector2 mp = GetMousePosition();
        Rectangle bounds = {position.x, position.y, size.x, size.y};

        // Focus handling
        if (CheckCollisionPointRec(mp, bounds) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            focused = true;
        }
        else if (!CheckCollisionPointRec(mp, bounds) &&
                 IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            focused = false;
        }

        // Text input (IP format only)
        if (focused)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((isdigit(key) || key == '.') && text.length() < 15)
                {
                    text += (char)key;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && !text.empty())
            {
                text.pop_back();
            }
        }
    }

    void draw() override
    {
        // Box background
        DrawRectangleRounded({position.x, position.y, size.x, size.y}, 0.3f, 12, color);

        // Text + cursor
        Vector2 textPos = {position.x + 20, position.y + (size.y / 2 - 15)};
        DrawTextEx(UIComponent::primaryFont, text.c_str(), textPos, 30, 1, WHITE);

        // Blinking cursor
        if (focused && (cursorBlink++ % 60 < 30))
        {
            float cursorX = textPos.x + MeasureTextEx(UIComponent::primaryFont, text.c_str(), 30, 1).x + 2;
            DrawLine(cursorX, textPos.y, cursorX, textPos.y + 30, WHITE);
        }
    }

    string getText() { return text; }
};
// image component class
class ImageComponent : public UIComponent
{
    friend class Container;
    Texture2D texture;
    string path;

public:
    ImageComponent(Vector2 pos, Vector2 sz, const string &imgPath)
        : UIComponent(pos, sz, BLANK), path(imgPath)
    {
        Image img = LoadImage(path.c_str());
        texture = LoadTextureFromImage(img);
        UnloadImage(img);
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR); // Crisp scaling
        updatePosition();
    }

    void updatePosition() override
    {
        UIComponent::updatePosition();
        // Maintain aspect ratio
        float scale = min(size.x / texture.width, size.y / texture.height);
        size = {texture.width * scale, texture.height * scale};
    }

    void draw() override
    {
        DrawTexturePro(texture,
                       {0, 0, (float)texture.width, (float)texture.height},
                       {position.x, position.y, size.x, size.y},
                       {0, 0}, 0, WHITE);
        // Add border
        DrawRectangleLinesEx(
            {position.x - 2, position.y - 2, size.x + 4, size.y + 4},
            2, BLACK);
    }

    ~ImageComponent()
    {
        UnloadTexture(texture);
    }
    void handleEvent() override {}
};

// Container Class
class Container : public UIComponent
{
    vector<UIComponent *> comps;
    Label *title;

public:
    Container(Vector2 pos, Vector2 sz, Color col, string titleText, int titleSize = 80, Color titleColor = WHITE, float topOffset = 94)
        : UIComponent(pos, sz, col)
    {
        type = CONTAINER;
        title = new Label({pos.x + sz.x / 2, pos.y + topOffset}, titleText, titleSize, titleColor, true);
        updatePosition();
    }
    ~Container() { delete title; }
    void addInputBox(InputBox *input, Vector2 relPos)
    {
        input->originalPosition = {originalPosition.x + relPos.x,
                                   originalPosition.y + relPos.y};
        input->updatePosition();
        comps.push_back(input);
    }
    void addButton(Button *btn, Vector2 relPos)
    {
        btn->originalPosition = {originalPosition.x + relPos.x, originalPosition.y + relPos.y};
        btn->updatePosition();
        comps.push_back(btn);
    }
    void addLabel(Label *lbl, Vector2 relPos)
    {
        lbl->originalPosition = {originalPosition.x + relPos.x, originalPosition.y + relPos.y};
        lbl->updatePosition();
        comps.push_back(lbl);
    }
    void addImage(ImageComponent *img, Vector2 relPos)
    {
        img->originalPosition = {originalPosition.x + relPos.x,
                                 originalPosition.y + relPos.y};
        img->updatePosition();
        comps.push_back(img);
    }

    void updatePosition() override
    {
        UIComponent::updatePosition();
        title->updatePosition();
        for (auto b : comps)
            b->updatePosition();
    }

    void draw() override
    {
        DrawRectangleRounded(Rectangle{position.x, position.y, size.x, size.y}, 0.15f, 32, color);

        title->draw();
        for (auto b : comps)
            b->draw();
    }

    void handleEvent() override
    {
        for (auto b : comps)
            b->handleEvent();
    }
};

// Background Class
// Modified Background class
class Background : public UIComponent
{
private:
    Ball b1, b2;

public:
    Background(Vector2 pos, Vector2 sz, Color col)
        : UIComponent(pos, sz, col)
    {

        b1 = Ball(20, 300, 20, 10, Color{186, 58, 62, 255});
        b2 = Ball(1020, 1600, 20, 10, Color{255, 183, 3, 255});
        type = CONTAINER;
    }

    void updatePosition() override
    {
        UIComponent::updatePosition();
        b1.updatePosition();
        b2.updatePosition();
    }

    void draw() override
    {

        DrawRectangle(position.x, position.y, size.x, size.y, color);
        b1.Draw();
        b2.Draw();
    }

    void handleEvent() override
    {
        b1.Update();
        b2.Update();
    }
};

class ButtonJoin : public Button
{
    using Button::Button;

public:
    void click() override
    {
        // Access input box text (you'll need to make ipInput accessible)
        cout << "Attempting to join: " << globalIPInput->getText() << endl;
        runOnline = 1;
    }
};

class ButtonCreate : public Button
{
    using Button::Button;

public:
    void click() override
    {
        cout << "Server created! IP: " << endl;
        isServer = true;
        runOnline = 1;
    }
};
class ThemeFireIce : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentTheme = THEME1;
        currentState = LOBBY;
    }
};

class ThemeForest : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentTheme = THEME2;
        currentState = LOBBY;
    }
};

class ThemeNeon : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentTheme = THEME3;
        currentState = LOBBY;
    }
};

class ThemeFuturistic : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentTheme = THEME4;
        currentState = LOBBY;
    }
};

class ThemeUnderwater : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentTheme = THEME5;
        currentState = LOBBY;
    }
};

class ThemeSpace : public Button
{
public:
    using Button::Button;
    void click() override
    {
        currentTheme = THEME6;
        currentState = LOBBY;
    }
};

// below is run client to run the client
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
    scoreBoardC score;
    int oldSW = screenWidth, oldSH = screenHeight;
    state ballCState = {(float)screenWidth / 2, (float)screenHeight / 2, (float)screenHeight / 2, (float)screenHeight / 2};
    int signX = 1;
    int signY = 1;
    const double DELAY = 0.1;
    double now = GetTime();

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight}; // Play area starts at Y=25
    themeC classic(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");

    paddleC left(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    paddleC right(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                  (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

    ballC gameballC((ballCState.x * ((float)screenWidth / oldSW)), (ballCState.y * ((float)screenHeight / oldSH)),
                    (screenWidth * 0.02f), classic.getballCColor(),
                    (screenWidth * 0.007f), (screenHeight * 0.005f), mode + themePath + "ball.png");

    Color buttonColor = WHITE;
    int frameNo = 0;
    int scoreLeft = 0;
    int scoreRight = 0;

    while (!WindowShouldClose())
    {
        // Handle window resize
        if (IsWindowResized() || !frameNo)
        {
            oldSW = screenWidth;
            oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
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
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&right) paddleC(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&gameballC) ballC((int)newballCX, (int)newballCY, newballCRadius, classic.getballCColor(),
                                   (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + themePath + "ball.png");

            new (&classic) themeC(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");
            networkReceiveScores(host, &scoreLeft, &scoreRight);
            *(score.getScore1()) = scoreLeft;
            *(score.getScore2()) = scoreRight;
        }

        // Pause button interaction
        Vector2 mousePos = GetMousePosition();
        Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
        bool isHover = CheckCollisionPointRec(mousePos, button);
        isPaused = checkPauseC(isHover, &buttonColor, peer, host);
        Image paused = LoadImage("Assets/winningPage/paused.jpg");
        Texture2D pausedTexture = LoadTextureFromImage(paused);

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
            left.drawpaddleC();
            right.drawpaddleC();
            gameballC.drawballC();

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
            (Rectangle){0, 0, (float)pausedTexture.width, (float)pausedTexture.height-30},
            (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()-30},
            (Vector2){0, 0},
            0.0f,
            WHITE);

            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
    }
    if ((int)frameNo / 60 >= 59)
    {
        if (*(score.getScore1()) > *(score.getScore2()))
        {
            return 1;
        }
        else if (*(score.getScore1()) < *(score.getScore2()))
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
    networkShutdown(host);
    return 0;
}

// below is run server to run the server
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

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight};
    themeS classic(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");
    paddleS left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    paddleS right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE, (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    scoreBoardS score;

    ballS gameballS(oldSW / 2, oldSH / 2, (screenWidth * 0.02f), classic.getballSColor(), (screenWidth * 0.007f), (screenHeight * 0.005f), mode + themePath + "ball.png");
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
            int signSpeedX = (gameballS.getballSSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameballS.getballSSpeedY() < 0) ? -1 : 1;

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

            classic.~themeS();
            gameballS.~ballS();
            left.~paddleS();
            right.~paddleS();
            new (&left) paddleS(classic.getBorderWidth() + 5, screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&right) paddleS(screenWidth - 10 - (int)(screenWidth * 0.02f), screenHeight / 2 - (int)(screenHeight * 0.165f / 2), WHITE,
                                 (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");

            new (&gameballS) ballS((int)newballSX, (int)newballSY, newballSRadius, classic.getballSColor(),
                                   (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + themePath + "ball.png");

            new (&classic) themeS(RED, BLACK, YELLOW, border, 5, mode + themePath + "background.png");
            networkSendScore(host, NULL, *(score.getScore1()), *(score.getScore2()));
        }

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPauseS(isHover, &buttonColor, host);

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
            networkReceiveState(host, &dummy.x, &dummy.y, &dummy.p1, &dummy.p2);
            cout << "Reaceived p2 as" << dummy.p2 << endl;
            dummy.p2 *= screenHeight;
            if (!((std::isinf(dummy.p2) || std::isnan(dummy.p2))) && dummy.p2 >= 0 && dummy.p2 <= screenHeight)
                right.setPositionY((int)dummy.p2);

            gameballS.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());
            left.update();
            right.update();

            networkSendState(host, NULL, (float)gameballS.getPositionX() / screenWidth,
                             (float)gameballS.getPositionY() / screenHeight, (float)left.getPositionY() / screenHeight, (float)right.getPositionY());

            BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(scoreChoice);
            left.drawpaddleS();
            right.drawpaddleS();
            gameballS.drawballS();
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
            (Rectangle){0, 0, (float)pausedTexture.width, (float)pausedTexture.height-30},
            (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()-30},
            (Vector2){0, 0},
            0.0f,
            WHITE);

            EndDrawing();
        }
        isPaused = receivePause(host, isPaused);
        if ((int)frameNo / 60 >= 59)
        {
            return 0;
        }
    }
    UnloadTexture(pausedTexture);
    networkShutdown(host);
    return 0;
}

int offlinePong(THEMES currTheme, int robotMode)
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
        scoreChoice = 3;
    }
    if (currTheme == THEME6)
    {
        themePath = "spacegalaxy/";
        scoreChoice = 1;
    }

    Rectangle border = {0, 25, (float)screenWidth, (float)screenHeight};
    themeOffline classic(RED, BLACK, border, 5, mode + themePath + "background.png");
    paddleOffline left(10, screenHeight / 2 - (int)(screenHeight * 0.165f / 2),
                       (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    paddleOffline right(screenWidth - 30, screenHeight / 2 - (int)(screenHeight * 0.165f / 2),
                        (int)(screenHeight * 0.165f), (int)(screenWidth * 0.02f), mode + themePath + "paddle.png");
    scoreBoardOffline score;

    ballOffline gameballS(screenWidth / 2, screenHeight / 2, (screenWidth * 0.02f),
                          (screenWidth * 0.007f), (screenHeight * 0.005f), mode + themePath + "ball.png");
    Rectangle button = {(float)screenWidth - 70, 5, 60, 15};
    Color buttonColor = WHITE;
    int frameNo = 0;
    Image paused = LoadImage("Assets/winningPage/paused.jpg");
    Texture2D pausedTexture = LoadTextureFromImage(paused);

    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            int oldSW = screenWidth;
            int oldSH = screenHeight;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight() - 30;
            int signSpeedX = (gameballS.getBallSSpeedX() < 0) ? -1 : 1;
            int signSpeedY = (gameballS.getBallSSpeedY() < 0) ? -1 : 1;
            float ballX_ratio = (float)gameballS.getPositionX() / oldSW;
            float ballY_ratio = (float)gameballS.getPositionY() / oldSH;
            int newBallRadius = (int)(screenWidth * 0.02f);
            int newBallX = (int)(ballX_ratio * screenWidth);
            int newBallY = (int)(ballY_ratio * screenHeight);
            newBallX = Clamp(newBallX, newBallRadius, screenWidth - newBallRadius);
            newBallY = Clamp(newBallY, 25 + newBallRadius, screenHeight - newBallRadius - 25);
            int paddleHeight = (int)(screenHeight * 0.165f);
            int paddleWidth = (int)(screenWidth * 0.02f);
            classic.~themeOffline();
            left.~paddleOffline();
            right.~paddleOffline();
            gameballS.~ballOffline();
            border = {0, 25, (float)screenWidth, (float)screenHeight};
            new (&classic) themeOffline(RED, BLACK, border, 5, mode + themePath + "background.png");
            new (&left) paddleOffline(10, screenHeight / 2 - paddleHeight / 2, paddleHeight, paddleWidth, mode + themePath + "paddle.png");
            new (&right) paddleOffline(screenWidth - 10 - paddleWidth, screenHeight / 2 - paddleHeight / 2, paddleHeight, paddleWidth, mode + themePath + "paddle.png");
            new (&gameballS) ballOffline(newBallX, newBallY, newBallRadius, (int)(signSpeedX * screenWidth * 0.007f), (int)(signSpeedY * screenHeight * 0.005f), mode + themePath + "ball.png");
            button = {(float)screenWidth - 70, 5, 60, 15};
        }

        Vector2 pos = GetMousePosition();
        bool isHover = CheckCollisionPointRec(pos, button);
        isPaused = checkPauseOffline(isHover, &buttonColor);

        if (!isPaused)
        {
            left.update(true);

            if (robotMode == 0)
            {
                int ballY = gameballS.getPositionY();
                int paddleCenter = right.getPositionY() + right.getRec().height / 2;
                float moveStep = screenHeight * 0.01f;
                if (paddleCenter < ballY - 5)
                {
                    right.setPositionY(right.getPositionY() + moveStep);
                }
                else if (paddleCenter > ballY + 5)
                {
                    right.setPositionY(right.getPositionY() - moveStep);
                }
                right.setPositionY(Clamp(right.getPositionY(), 25.0f, (float)screenHeight - right.getRec().height + 25));
            }
            else
            {
                right.update(false);
            }

            gameballS.update(left.getRec(), right.getRec(), score.getScore1(), score.getScore2());

            BeginDrawing();
            classic.drawBoard(screenWidth, screenHeight);
            score.drawBoard(scoreChoice);
            left.drawpaddleS();
            right.drawpaddleS();
            gameballS.drawballS();
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
            (Rectangle){0, 0, (float)pausedTexture.width, (float)pausedTexture.height-30},
            (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()-30},
            (Vector2){0, 0},
            0.0f,
            WHITE);

            EndDrawing();
        }
        if ((int)frameNo / 60 >= 59)
        {
            return 0;
        }
    }
    UnloadTexture(pausedTexture);
    return 0;
}

int winningPage(int endVal, MODES currentMode)
{
    Image image = LoadImage("Assets/winningPage/over.jpg");
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    bool isHome = false;
    bool shouldExit = false;

    while (!shouldExit)
    {
        BeginDrawing();
        ClearBackground((Color){58, 84, 59, 255});

        DrawTexturePro(
            texture,
            (Rectangle){0, 0, (float)texture.width, (float)texture.height},
            (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
            (Vector2){0, 0},
            0.0f,
            WHITE);

        if (IsKeyPressed(KEY_H))
        {
            isHome = true;
            shouldExit = true;
        }
        else if (IsKeyPressed(KEY_ESCAPE))
        {
            isHome = false;
            shouldExit = true;
        }

        EndDrawing();
    }

    UnloadTexture(texture);
    return isHome ? 1 : 0;
}

void mainMenu()
{
    const int baseWidth = 800;
    const int baseHeight = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(baseWidth, baseHeight, "PONG BALL GAME MENU");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    ScreenBounds = Rectangle{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};

    UIComponent::LoadFonts("Assets/fonts/Rajdhani-SemiBold.ttf", "assets/fonts/LuckiestGuy-Regular.ttf");
    SetTextureFilter(UIComponent::primaryFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(UIComponent::secondaryFont.texture, TEXTURE_FILTER_BILINEAR);

    vector<UIComponent *> components, lobbyComponents, modeComponents, aboutComponents, onlineModeComponents, themeComponents;

    Container mainMenu(
        {baseWidth / 2 - 450, baseHeight / 2 - 350},
        {900, 700},
        CONTAINER_COLOR,
        "MAIN MENU",
        80,
        HeadingColor

    );

    // Create buttons with theme colors
    ButtonStart startBtn({0, 0}, {600, 80}, BUTTON_ORANGE, "START GAME", 40, WHITE,
                         BUTTON_ORANGE_HOVER, BUTTON_ORANGE_PRESSED);

    ButtonMode modeBtn({0, 0}, {600, 80}, BUTTON_CERULEAN, "SELECT MODE", 40, WHITE,
                       BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);

    ButtonTheme themeBtn({0, 0}, {600, 80}, BUTTON_RUST, "SELECT THEME", 40, WHITE,
                         BUTTON_RUST_HOVER, BUTTON_RUST_PRESSED);

    ButtonAbout aboutBtn({0, 0}, {600, 80}, BUTTON_FOREST, "ABOUT DEVELOPERS", 40, WHITE,
                         BUTTON_FOREST_HOVER, BUTTON_FOREST_PRESSED);

    ButtonQuit quitBtn({0, 0}, {600, 80}, BUTTON_CARMINE, "QUIT GAME", 40, WHITE,
                       BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);

    // Add buttons to main menu
    mainMenu.addButton(&startBtn, {150, 160});
    mainMenu.addButton(&modeBtn, {150, 260});
    mainMenu.addButton(&themeBtn, {150, 360});
    mainMenu.addButton(&aboutBtn, {150, 460});
    mainMenu.addButton(&quitBtn, {150, 560});

    // Mode Selection Container
    Container modeMenu(
        {baseWidth / 2 - 400, baseHeight / 2 - 300},
        {800, 600},
        CONTAINER_COLOR,
        "MODE SELECTION",
        60,
        HeadingColor);

    // Mode selection buttons
    ButtonOnline onlineBtn({0, 0}, {600, 80}, BUTTON_ORANGE, "ONLINE MODE", 40, WHITE,
                           BUTTON_ORANGE_HOVER, BUTTON_ORANGE_PRESSED);

    ButtonOffline offlineBtn({0, 0}, {600, 80}, BUTTON_CERULEAN, "OFFLINE MODE", 40, WHITE,
                             BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);

    ButtonComputer computerBtn({0, 0}, {600, 80}, BUTTON_FOREST, "COMPUTER MODE", 40, WHITE,
                               BUTTON_FOREST_HOVER, BUTTON_FOREST_PRESSED);

    ButtonClose backBtn({0, 0}, {600, 80}, BUTTON_CARMINE, "BACK", 40, WHITE,
                        BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);

    // Add buttons to mode menu
    modeMenu.addButton(&onlineBtn, {100, 150});
    modeMenu.addButton(&offlineBtn, {100, 250});
    modeMenu.addButton(&computerBtn, {100, 350});
    modeMenu.addButton(&backBtn, {100, 450});

    // Add containers to components
    lobbyComponents.push_back(&mainMenu);
    modeComponents.push_back(&modeMenu);
    components.push_back(&mainMenu);
    components.push_back(&modeMenu);

    // Background setup
    Background bg({0, 0}, {baseWidth, baseHeight}, BACKGROUND_COLOR);
    components.push_back(&bg);

    // SCREEN OF THE ABOUT DEVELOPERS
    //  Button(Vector2 pos, Vector2 sz, Color col,
    //  string txt, int fontSize, Color textCol,
    //  Color hoverCol, Color pressCol)
    Container aboutDevContainer(
        {baseWidth / 2 - 450, baseHeight / 2 - 350},
        {900, 700},
        CONTAINER_COLOR,
        "WHO ARE WE?",
        58,
        HeadingColor,
        70);

    Label dev1Label({0, 0}, "MUZAMIL SULEMAN", 50, TextColor);
    ButtonViewProfile dev1Button({0, 0}, {600, 80}, BUTTON_RUST, "VIEW PROFILE", 36, WHITE,
                                 BUTTON_RUST_HOVER, BUTTON_RUST_PRESSED);
    dev1Button.setLink("https://github.com/MUZAMILALISULEMAN");
    Label dev2Label({0, 0}, "SHAMVEEL KHAN", 50, TextColor);
    ButtonViewProfile dev2Button({0, 0}, {600, 80}, BUTTON_RUST, "VIEW PROFILE", 36, WHITE,
                                 BUTTON_RUST_HOVER, BUTTON_RUST_PRESSED);
    dev2Button.setLink("https://github.com/Shamveel-Khan");
    Label dev3Label({0, 0}, "KABEER JAVED", 50, TextColor);
    ButtonViewProfile dev3Button({0, 0}, {600, 80}, BUTTON_RUST, "VIEW PROFILE", 36, WHITE,
                                 BUTTON_RUST_HOVER, BUTTON_RUST_PRESSED);
    dev3Button.setLink("https://github.com/Kabeer0700-gif");
    // Back button (using existing carmine colors)
    ButtonClose backBtnAbout({0, 0}, {200, 80}, BUTTON_CARMINE, "BACK", 36, WHITE,
                             BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);

    // Positioning remains identical
    aboutDevContainer.addLabel(&dev1Label, {450, 135});
    aboutDevContainer.addButton(&dev1Button, {150, 175});
    aboutDevContainer.addLabel(&dev2Label, {450, 298});
    aboutDevContainer.addButton(&dev2Button, {150, 335});
    aboutDevContainer.addLabel(&dev3Label, {450, 458});
    aboutDevContainer.addButton(&dev3Button, {150, 495});
    aboutDevContainer.addButton(&backBtnAbout, {350, 595});

    aboutComponents.push_back(&aboutDevContainer);
    components.push_back(&aboutDevContainer);

    // ONLINE GAME SETUP
    Container onlineMenu(
        {baseWidth / 2 - 400, baseHeight / 2 - 300},
        {800, 600},
        CONTAINER_COLOR,
        "ONLINE MODE",
        60,
        HeadingColor);

    // Online mode components
    InputBox ipInput({0, 0}, {600, 60}, GRAY);

    ButtonJoin joinBtn({0, 0}, {600, 80}, BUTTON_CERULEAN, "JOIN SERVER", 40, WHITE,
                       BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);
    ButtonCreate createBtn({0, 0}, {600, 80}, BUTTON_FOREST, "CREATE SERVER", 40, WHITE,
                           BUTTON_FOREST_HOVER, BUTTON_FOREST_PRESSED);
    ButtonClose onlineBackBtn({0, 0}, {600, 80}, BUTTON_CARMINE, "BACK", 40, WHITE,
                              BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);
    globalIPInput = &ipInput;

    // Setup online menu

    onlineMenu.addInputBox(&ipInput, {100, 150});
    onlineMenu.addButton(&joinBtn, {100, 250});
    onlineMenu.addButton(&createBtn, {100, 350});
    onlineMenu.addButton(&onlineBackBtn, {100, 450});
    onlineModeComponents.push_back(&onlineMenu);
    components.push_back(&onlineMenu);

    // Theme Selection Container
    Container themeMenu(
        {baseWidth / 2 - 400, baseHeight / 2 - 400},
        {800, 800},
        CONTAINER_COLOR,
        "SELECT THEME",
        70,
        HeadingColor);

    // Theme 1: Fire & Ice (Orange Theme)
    ImageComponent theme1Img({0, 0}, {200, 150}, "assets/themes/fire_ice.png");
    ThemeFireIce theme1Btn({0, 0}, {200, 60},
                           BUTTON_ORANGE, "ICE AND FIRE", 30, WHITE,
                           BUTTON_ORANGE_HOVER, BUTTON_ORANGE_PRESSED);

    // Theme 2: Forest (Green Theme)
    ImageComponent theme2Img({0, 0}, {200, 150}, "assets/themes/forest.png");
    ThemeForest theme2Btn({0, 0}, {200, 60},
                          BUTTON_FOREST, "FOREST", 30, WHITE,
                          BUTTON_FOREST_HOVER, BUTTON_FOREST_PRESSED);

    // Theme 3: Neon (Cerulean Blue Theme)
    ImageComponent theme3Img({0, 0}, {200, 150}, "assets/themes/neon.png");
    ThemeNeon theme3Btn({0, 0}, {200, 60},
                        BUTTON_CERULEAN, "NEON", 30, WHITE,
                        BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);

    // Theme 4: Futuristic (Rust Theme)
    ImageComponent theme4Img({0, 0}, {200, 150}, "assets/themes/futuristic.png");
    ThemeFuturistic theme4Btn({0, 0}, {200, 60},
                              BUTTON_RUST, "FUTURISTIC", 30, WHITE,
                              BUTTON_RUST_HOVER, BUTTON_RUST_PRESSED);

    // Theme 5: Underwater (Cerulean Blue Theme)
    ImageComponent theme5Img({0, 0}, {200, 150}, "assets/themes/underwater.png");
    ThemeUnderwater theme5Btn({0, 0}, {200, 60},
                              BUTTON_CERULEAN, "UNDERWATER", 30, WHITE,
                              BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);

    // Theme 6: Space (Carmine Red Theme)
    ImageComponent theme6Img({0, 0}, {200, 150}, "assets/themes/space.png");
    ThemeSpace theme6Btn({0, 0}, {200, 50},
                         BUTTON_CARMINE, "SPACE", 30, WHITE,
                         BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);

    // Back Button (Consistent with other screens)
    ButtonClose themeBackBtn({0, 0}, {300, 50},
                             BUTTON_CARMINE, "BACK", 34, WHITE,
                             BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);

    // Grid layout parameters
    const Vector2 startPos = {50, 160};
    const float imageHeight = 150;
    const float buttonYOffset = 160; // 150px image height + 10px spacing
    const float colSpacing = 250;
    const float rowSpacing = 250; // 300px row spacing + 10px buffer

    // Row 1
    // Theme 1 - Fire & Ice
    themeMenu.addImage(&theme1Img, {startPos.x, startPos.y});
    themeMenu.addButton(&theme1Btn, {startPos.x, startPos.y + buttonYOffset});

    // Theme 2 - Forest
    themeMenu.addImage(&theme2Img, {startPos.x + colSpacing, startPos.y});
    themeMenu.addButton(&theme2Btn, {startPos.x + colSpacing, startPos.y + buttonYOffset});

    // Theme 3 - Neon
    themeMenu.addImage(&theme3Img, {startPos.x + 2 * colSpacing, startPos.y});
    themeMenu.addButton(&theme3Btn, {startPos.x + 2 * colSpacing, startPos.y + buttonYOffset});

    // Row 2
    // Theme 4 - Futuristic
    int padding = 37; // Padding for the second row
    themeMenu.addImage(&theme4Img, {startPos.x, startPos.y + rowSpacing + padding});
    themeMenu.addButton(&theme4Btn, {startPos.x, startPos.y + rowSpacing + buttonYOffset + padding});

    // Theme 5 - Underwater
    themeMenu.addImage(&theme5Img, {startPos.x + colSpacing, startPos.y + rowSpacing + padding});
    themeMenu.addButton(&theme5Btn, {startPos.x + colSpacing, startPos.y + rowSpacing + buttonYOffset + padding});

    // Theme 6 - Space
    themeMenu.addImage(&theme6Img, {startPos.x + 2 * colSpacing, startPos.y + rowSpacing + padding});
    themeMenu.addButton(&theme6Btn, {startPos.x + 2 * colSpacing, startPos.y + rowSpacing + buttonYOffset + padding});

    // Back button (centered below grid)
    const float backButtonY = startPos.y + rowSpacing + imageHeight + 100;
    themeMenu.addButton(&themeBackBtn, {250, backButtonY + padding + 13});

    themeComponents.push_back(&themeMenu);
    components.push_back(&themeMenu);

    // Main loop
    while (!WindowShouldClose())
    {

        float sx = GetScreenWidth() / (float)baseWidth;
        float sy = GetScreenHeight() / (float)baseHeight;

        if (GetScreenWidth() > 0 && GetScreenHeight() > 0 &&
            (sx != UIComponent::scaleX || sy != UIComponent::scaleY))
        {
            ScreenBounds = Rectangle{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
            UIComponent::setScale(sx, sy);
            for (auto c : components)
            {
                c->updatePosition();
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentState)
        {
        case LOBBY:
            bg.handleEvent();
            bg.draw();
            for (auto c : lobbyComponents)
            {
                c->handleEvent();
                c->draw();
            }

            break;
        case MODE:
            bg.handleEvent();
            bg.draw();
            for (auto c : modeComponents)
            {
                c->handleEvent();
                c->draw();
            }
            break;
        case GAMEPLAY:
            int endVal, replayVal;
            if (currentMode == OFFLINE)
            {
                endVal = offlinePong(currentTheme, 1);
            }
            if (currentMode == COMPUTER)
            {
                endVal = offlinePong(currentTheme, 0);
            }
            replayVal = winningPage(endVal, currentMode);
            if (replayVal)
            {
                currentState = LOBBY;
            }
            else
            {
                return;
            }
            break;
        case ABOUT:
            bg.handleEvent();
            bg.draw();
            for (auto c : aboutComponents)
            {
                c->handleEvent();
                c->draw();
            }
            break;
        case ONLINEGAME:
            bg.handleEvent();
            bg.draw();
            for (auto c : onlineModeComponents)
            {
                c->handleEvent();
                c->draw();
            }
            if (!isServer && runOnline)
            {
                endVal = runClient(currentTheme, globalIPInput->getText());
            }
            else if (runOnline)
            {
                endVal = runServer(currentTheme);
            }
            break;
        case THEME:
            bg.handleEvent();
            bg.draw();
            for (auto c : themeComponents)
            {
                c->handleEvent();
                c->draw();
            }
            break;
        case EXIT:
            CloseWindow();
            break;
        }
        DrawFPS(20, 20);
        EndDrawing();
    }
    UIComponent::UnloadFonts();
}
int main()
{
    mainMenu();
}