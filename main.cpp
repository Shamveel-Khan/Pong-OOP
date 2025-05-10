#include <cstdlib>
#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
using namespace std;
Rectangle ScreenBounds;

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
    GAMEPLAY,
    EXIT,
    THEME,
    ABOUT,
    ONLINEGAME
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

THEMES currentTheme = THEME1;
GAMESTATES currentState = LOBBY;
MODES currentMode = OFFLINE;
// helper function to redirect to links
void openLink(const string link)
{
    system(("start " + link).c_str());
}
string GetIPAddress(){}
class InputBox;
class Container;
class Background;
InputBox* globalIPInput = nullptr;

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
class ImageComponent : public UIComponent {
    friend class Container; 
    Texture2D texture;
    string path;

public:
    ImageComponent(Vector2 pos, Vector2 sz, const string& imgPath) 
        : UIComponent(pos, sz, BLANK), path(imgPath) {
        Image img = LoadImage(path.c_str());
        texture = LoadTextureFromImage(img);
        UnloadImage(img);
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR); // Crisp scaling
        updatePosition();
    }

    void updatePosition() override {
        UIComponent::updatePosition();
        // Maintain aspect ratio
        float scale = min(size.x/texture.width, size.y/texture.height);
        size = {texture.width*scale, texture.height*scale};
    }

    void draw() override {
        DrawTexturePro(texture,
            {0,0,(float)texture.width,(float)texture.height},
            {position.x, position.y, size.x, size.y},
            {0,0}, 0, WHITE);
        // Add border
        DrawRectangleLinesEx(
            {position.x-2, position.y-2, size.x+4, size.y+4},
            2, BLACK
        );
    }
    
    ~ImageComponent() {
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
    void addImage(ImageComponent* img, Vector2 relPos) {
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
    }
};

class ButtonCreate : public Button
{
    using Button::Button;

public:
    void click() override
    {
        cout << "Server created! IP: " << GetIPAddress() << endl; // Raylib function
    }
};
class ThemeFireIce : public Button {
    public:
        using Button::Button;
        void click() override { currentTheme = THEME1;currentState = LOBBY; }
    };
    
    class ThemeForest : public Button {
    public:
        using Button::Button;
        void click() override { currentTheme = THEME2;currentState = LOBBY; }
    };
    
    class ThemeNeon : public Button {
    public:
        using Button::Button;
        void click() override { currentTheme = THEME3;currentState = LOBBY; }
    };
    
    class ThemeFuturistic : public Button {
    public:
        using Button::Button;
        void click() override { currentTheme = THEME4;currentState = LOBBY; }
    };
    
    class ThemeUnderwater : public Button {
    public:
        using Button::Button;
        void click() override { currentTheme = THEME5;currentState = LOBBY; }
    };
    
    class ThemeSpace : public Button {
    public:
        using Button::Button;
        void click() override { currentTheme = THEME6;currentState = LOBBY; }
    };


    


void mainMenu()
{
    const int baseWidth = 800;
    const int baseHeight = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(baseWidth,baseHeight, "PONG BALL GAME MENU");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    ScreenBounds = Rectangle{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};

    UIComponent::LoadFonts("Assets/fonts/Rajdhani-SemiBold.ttf", "assets/fonts/LuckiestGuy-Regular.ttf");
    SetTextureFilter(UIComponent::primaryFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(UIComponent::secondaryFont.texture, TEXTURE_FILTER_BILINEAR);

    vector<UIComponent *> components, lobbyComponents, modeComponents, aboutComponents, onlineModeComponents,themeComponents;

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
    {baseWidth/2 - 400, baseHeight/2 - 400},
    {800, 800},
    CONTAINER_COLOR,
    "SELECT THEME",
    70,
    HeadingColor
);

// Theme 1: Fire & Ice (Orange Theme)
ImageComponent theme1Img({0,0}, {200,150}, "assets/themes/fire_ice.png");
ThemeFireIce theme1Btn({0,0}, {200,60}, 
    BUTTON_ORANGE, "ICE AND FIRE", 30, WHITE,
    BUTTON_ORANGE_HOVER, BUTTON_ORANGE_PRESSED);

// Theme 2: Forest (Green Theme)
ImageComponent theme2Img({0,0}, {200,150}, "assets/themes/forest.png");
ThemeForest theme2Btn({0,0}, {200,60}, 
    BUTTON_FOREST, "FOREST", 30, WHITE,
    BUTTON_FOREST_HOVER, BUTTON_FOREST_PRESSED);

// Theme 3: Neon (Cerulean Blue Theme)
ImageComponent theme3Img({0,0}, {200,150}, "assets/themes/neon.png");
ThemeNeon theme3Btn({0,0}, {200,60}, 
    BUTTON_CERULEAN, "NEON", 30, WHITE,
    BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);

// Theme 4: Futuristic (Rust Theme)
ImageComponent theme4Img({0,0}, {200,150}, "assets/themes/futuristic.png");
ThemeFuturistic theme4Btn({0,0}, {200,60}, 
    BUTTON_RUST, "FUTURISTIC", 30, WHITE,
    BUTTON_RUST_HOVER, BUTTON_RUST_PRESSED);

// Theme 5: Underwater (Cerulean Blue Theme)
ImageComponent theme5Img({0,0}, {200,150}, "assets/themes/underwater.png");
ThemeUnderwater theme5Btn({0,0}, {200,60}, 
    BUTTON_CERULEAN, "UNDERWATER", 30, WHITE,
    BUTTON_CERULEAN_HOVER, BUTTON_CERULEAN_PRESSED);

// Theme 6: Space (Carmine Red Theme)
ImageComponent theme6Img({0,0}, {200,150}, "assets/themes/space.png");
ThemeSpace theme6Btn({0,0}, {200,50}, 
    BUTTON_CARMINE, "SPACE", 30, WHITE,
    BUTTON_CARMINE_HOVER, BUTTON_CARMINE_PRESSED);

// Back Button (Consistent with other screens)
ButtonClose themeBackBtn({0,0}, {300,50}, 
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
                themeMenu.addImage(&theme3Img, {startPos.x + 2*colSpacing, startPos.y});
                themeMenu.addButton(&theme3Btn, {startPos.x + 2*colSpacing, startPos.y + buttonYOffset});
                
                // Row 2
                // Theme 4 - Futuristic
                int padding = 37; // Padding for the second row
                themeMenu.addImage(&theme4Img, {startPos.x, startPos.y + rowSpacing+padding});
                themeMenu.addButton(&theme4Btn, {startPos.x, startPos.y + rowSpacing + buttonYOffset+padding});
                
                // Theme 5 - Underwater
                themeMenu.addImage(&theme5Img, {startPos.x + colSpacing, startPos.y + rowSpacing+padding});
                themeMenu.addButton(&theme5Btn, {startPos.x + colSpacing, startPos.y + rowSpacing + buttonYOffset+padding});
                
                // Theme 6 - Space
                themeMenu.addImage(&theme6Img, {startPos.x + 2*colSpacing, startPos.y + rowSpacing+padding});
                themeMenu.addButton(&theme6Btn, {startPos.x + 2*colSpacing, startPos.y + rowSpacing + buttonYOffset+padding});
                
                // Back button (centered below grid)
                const float backButtonY = startPos.y + rowSpacing + imageHeight + 100;
                themeMenu.addButton(&themeBackBtn, {250, backButtonY+padding+13});

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
            DrawText("Gameplay Started!", baseWidth / 2 - 100, baseHeight / 2, 30, WHITE);
            if (IsKeyPressed(KEY_ESCAPE))
                currentState = LOBBY;
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
int main() {
    cout<<"\nRunning\n";
    mainMenu();
}