#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
using namespace std;

float baseWidth = 1920.0f;
float baseHeight = 1080.0f;
float buttonWidth = .75f * 600;
float buttonHeight = .75f * 160;
float SCALE;

enum gameMode { ONLINE, OFFLINE, COMPUTER };
gameMode gameMODE = OFFLINE;
enum gameTHEME { ICEANDFIRE, FOREST, UNDERWATER };

class Settings {
private:

    bool sound;
    bool music;

public:
    Settings() {
  
        sound = true;
        music = true;
    }

    Settings( bool soundOn, bool musicOn) {

        sound = soundOn;
        music = musicOn;
    }




    bool isSoundOn() const { return sound; }
    bool isMusicOn() const { return music; }

    void turnOnSound() { sound = true; }
    void turnOffSound() { sound = false; }
    void turnOnMusic() { music = true; }
    void turnOffMusic() { music = false; }
};

enum STATES { LOBBY, PLAY, EXIT, MODE, THEME, SETTINGS };
STATES gameSTATE = LOBBY;

Vector2 getScreenCenter(Vector2 size) {
    Vector2 coords = {(GetScreenWidth() - size.x) / 2, (GetScreenHeight() - size.y) / 2};
    return coords;
}

void calcScale() {
    float scaleX = GetScreenWidth() / baseWidth;
    float scaleY = GetScreenHeight() / baseHeight;
    SCALE = fmin(scaleX, scaleY);
}

class comp {
public:
    Vector2 coords;
    Texture2D imgTexture;

    comp() {
        imgTexture = {0};
        coords = {0, 0};
    }

    comp(Vector2 coords, Texture2D texture) : coords(coords), imgTexture(texture) {
        this->coords.x *= SCALE;
        this->coords.y *= SCALE;
    }

    comp(Texture2D texture) {
        imgTexture = texture;
        coords = {0, 0};
    }

    inline Vector2 getSize() const {
        return Vector2{float(imgTexture.width * SCALE), float(imgTexture.height * SCALE)};
    }

    Vector2 getCoords() const { return coords; }

    void setSize(Vector2 size) {
        imgTexture.width = size.x * SCALE;
        imgTexture.height = size.y * SCALE;
    }

    void setCoords(Vector2 coords) {
        this->coords.x = coords.x * SCALE;
        this->coords.y = coords.y * SCALE;
    }

    void setTexture(string path) {
        Image img = LoadImage(path.c_str());
        imgTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    virtual void draw() {
        if (imgTexture.id == 0) return;
        DrawTextureEx(imgTexture, coords, 0, SCALE, WHITE);
    }

    void draw(char c) {
        if (c != 'c') return;
        Vector2 COORDS = {
            float((GetScreenWidth() - (imgTexture.width*SCALE)) / 2),
            float((GetScreenHeight() - (imgTexture.height*SCALE)) / 2)};
        DrawTextureEx(imgTexture, COORDS, 0, SCALE, WHITE);
    }

    ~comp() {
        if (imgTexture.id != 0)
            UnloadTexture(imgTexture);
    }
};

class Button : public comp {
protected:
    Texture2D pressedImgTexture = {0};
    Texture2D unpressedImgTexture = {0};

public:
    Button() : comp() {}

    void setTexture(string unpressedPath, string pressedPath) {
        Image img1 = LoadImage(unpressedPath.c_str());
        Image img2 = LoadImage(pressedPath.c_str());

        unpressedImgTexture = LoadTextureFromImage(img1);
        pressedImgTexture = LoadTextureFromImage(img2);
        unpressedImgTexture.height = buttonHeight;
        unpressedImgTexture.width = buttonWidth;
        pressedImgTexture.height = buttonHeight;
        pressedImgTexture.width = buttonWidth;
        imgTexture = unpressedImgTexture;

        UnloadImage(img1);
        UnloadImage(img2);
    }

    void hover() {
        Rectangle bounds = {
            coords.x,
            coords.y,
            float(imgTexture.width * SCALE),
            float(imgTexture.height * SCALE)};

        if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
            imgTexture = pressedImgTexture;
    
        } else {
            imgTexture = unpressedImgTexture;
            
        }
    }

    virtual void click() = 0;

    void handleEvent() {
        draw();
        hover();
    
        Rectangle bounds = {
            coords.x,
            coords.y,
            float(imgTexture.width),
            float(imgTexture.height)
        };
    
        if (CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            click();
        }
    }
    

    ~Button() {
        if (pressedImgTexture.id != 0)
            UnloadTexture(pressedImgTexture);
        if (unpressedImgTexture.id != 0)
            UnloadTexture(unpressedImgTexture);
    }
};

class playButton : public Button {
public:
    void click() { gameSTATE = PLAY; }
};

class selectMode : public Button {
public:
    void click() { gameSTATE = MODE; }
};

class selectTheme : public Button {
public:
    void click() { gameSTATE = THEME; }
};

class settings : public Button {
public:
    void click() { gameSTATE = SETTINGS; }
};

class ExitButton : public Button {
public:
ExitButton() : Button(){}
    void click() { gameSTATE = EXIT; }
};

class DialogueBox : public comp {
protected:
    vector<Button *> Buttons;

public:
    DialogueBox() : comp() {}
    DialogueBox(Vector2 coords, Texture2D texture) : comp(coords, texture) {}

    virtual void calcPositions() = 0;
    virtual void setButton(Button *BTN) = 0;

    vector<Button*> getButtons() const { return Buttons; }

    void draw() {
        comp::draw('c');
        for (size_t i = 0; i < Buttons.size(); i++) {
            Buttons[i]->handleEvent();
        }
    }
};

class MenuDialogue : public DialogueBox {
public:
    MenuDialogue() : DialogueBox() {}
    MenuDialogue(Vector2 coords, Texture2D texture) : DialogueBox(coords, texture) {}

    void setButton(Button *BTN) {
        if (Buttons.size() < 5) {
            Buttons.push_back(BTN);
        }
    }

    virtual void calcPositions() {
        float xoffset = 21.3f + 700.0f;
        float yoffset = 67.0f + 152.0f;
        Vector2 SIZE = {buttonWidth, buttonHeight}; // unscaled — will be scaled inside setSize()
        Vector2 COORDS;
    
        for (size_t i = 0; i < Buttons.size(); i++) {
            Buttons[i]->setSize(SIZE);         // does scaling for size
            COORDS = {xoffset, yoffset};
            Buttons[i]->setCoords(COORDS);     // scaling applied here
            yoffset += (Buttons[i]->getSize().y / SCALE) + 67.0f;  // adjust using unscaled size
        }
    }
    
};

class ThemeDialogue : public DialogueBox {
private:
    vector<comp> themeImages;
    int imageCount;

public:
    ThemeDialogue() : DialogueBox(), imageCount(0) {}
    ThemeDialogue(Vector2 coords, Texture2D texture) : DialogueBox(coords, texture), imageCount(0) {}

    void setButton(Button *BTN) {
        if (Buttons.size() < 3) {
            Buttons.push_back(BTN);
        }
    }

    void setImage(string path) {
        if (themeImages.size() < 3) {
            comp newImage;
            newImage.setTexture(path);
            themeImages.push_back(newImage);
            imageCount++;
        }
    }

    vector<comp> getThemeImages() const { return themeImages; }

     void calcPositions() {

    }

    void draw() {
        DialogueBox::draw();
        for (unsigned int i = 0; i < themeImages.size(); i++) {
            themeImages[i].draw();
        }
    }
};
class ModeDialogue : public DialogueBox{

protected:

public:

void setButton(Button *BTN) {
    if (Buttons.size() < 3) {
        Buttons.push_back(BTN);
    }
}
virtual void calcPositions() {
    float xoffset = 40.0f + 700.0f;
    float yoffset = 60.0f + 325.0f ;
    Vector2 SIZE = {buttonWidth, buttonHeight}; // unscaled — will be scaled inside setSize()
    Vector2 COORDS;

    for (size_t i = 0; i < Buttons.size(); i++) {
        Buttons[i]->setSize(SIZE);         // does scaling for size
        COORDS = {xoffset, yoffset};
        Buttons[i]->setCoords(COORDS);     // scaling applied here
        yoffset += (Buttons[i]->getSize().y / SCALE) + 50.0f;  // adjust using unscaled size
    }
}



};

class onlineButton:public Button{
    public:
    void click(){
        gameMODE = ONLINE;
        gameSTATE = LOBBY;
    }
};
class offlineButton:public Button{
    public:
    void click(){
        gameMODE = OFFLINE;
        gameSTATE = LOBBY;
    }
};
class computerButton:public Button{
    public:
    void click(){
        gameMODE = COMPUTER;
        gameSTATE = LOBBY;
    }
};
void makeLobby(MenuDialogue& menu,
    playButton& playBTN,
    selectMode& modeBTN,
    selectTheme& themeBTN,
    settings& settingsBTN,
    ExitButton& exitBTN) 
{
    calcScale();
playBTN.setTexture("assets/menu/play_u.png", "assets/menu/play_p.png");
modeBTN.setTexture("assets/menu/mode_u.png", "assets/menu/mode_p.png");
settingsBTN.setTexture("assets/menu/settings_u.png", "assets/menu/settings_p.png");
themeBTN.setTexture("assets/menu/theme_u.png", "assets/menu/theme_p.png");
exitBTN.setTexture("assets/menu/exit_u.png", "assets/menu/exit_p.png");

menu.setTexture("assets/menu/mmenu.png");

menu.setButton(&playBTN);
menu.setButton(&modeBTN);
menu.setButton(&themeBTN);
menu.setButton(&settingsBTN);
menu.setButton(&exitBTN);

menu.calcPositions();

}

void initiateLobby(MenuDialogue &menu,comp &bg){
    bg.draw();
    menu.draw();

}

void makeModeDialogue(ModeDialogue &mode,onlineButton &btn1,offlineButton &btn2,computerButton &btn3){

mode.setTexture("assets/modeDialogue/mode.png");
btn1.setTexture("assets/modeDialogue/online_u.png","assets/modeDialogue/online_p.png");
btn2.setTexture("assets/modeDialogue/offline_u.png","assets/modeDialogue/offline_p.png");
btn3.setTexture("assets/modeDialogue/computer_u.png","assets/modeDialogue/computer_p.png");
mode.setButton(&btn1);
mode.setButton(&btn2);
mode.setButton(&btn3);
mode.calcPositions();

}
void initiateModeDialogue(ModeDialogue &mode,comp &bg){
    bg.draw();
    mode.draw();

}



int main() {
    InitWindow(1280.0f, 720.0f, "LOBBY SCREEN");
    SetTargetFPS(400);
    

    comp bg;
    MenuDialogue menu;
    playButton playBTN;
    selectMode modeBTN;
    selectTheme themeBTN;
    settings settingsBTN;
    ExitButton exitBTN;
    ModeDialogue mode;
    onlineButton onlineBTN;
    offlineButton offlineBTN;
    computerButton computerBTN;
    bg.setTexture("assets/bgg.png");


    makeLobby(menu, playBTN, modeBTN, themeBTN, settingsBTN, exitBTN);
    makeModeDialogue(mode,onlineBTN,offlineBTN,computerBTN);

    while (!WindowShouldClose()) {
        

        BeginDrawing();
        switch (gameSTATE) {
        case LOBBY:
        initiateLobby(menu,bg);

            break;
        case EXIT:
            CloseWindow();
            return 0;
        case PLAY:
            // Gameplay state here
            break;
        case MODE:
            initiateModeDialogue(mode,bg);
            
            break;
        case THEME:
            // Theme selection here
            break;
        case SETTINGS:
            // Settings dialog here
            break;
        }
    
        EndDrawing();

       
    }
    

    CloseWindow();
    return 0;
}
