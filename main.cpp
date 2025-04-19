#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
using namespace std;
float baseWidth = 1920.0f;
float baseHeight = 1080.0f;
float buttonWidth = 0.85f * 600;
float buttonHeight = 0.85f * 160;
float SCALE;
enum gameMode{
    ONLINE,
    OFFLINE,
    COMPUTER

};
enum gameTHEME{
    THEME1,
    THEME2,
    THEME3
};
class Settings {
    private:
        char paddleUp;
        char paddleDown;
        bool sound;
        bool music;
    
    public:
        
        Settings() {
            paddleUp = 'W';
            paddleDown = 'S';
            sound = true;
            music = true;
        }
    
        // Parameterized constructor
        Settings(char upKey, char downKey, bool soundOn, bool musicOn) {
            paddleUp = upKey;
            paddleDown = downKey;
            sound = soundOn;
            music = musicOn;
        }
    
        // Getters
        char getPaddleUp() const {
            return paddleUp;
        }
    
        char getPaddleDown() const {
            return paddleDown;
        }
    
        bool isSoundOn() const {
            return sound;
        }
    
        bool isMusicOn() const {
            return music;
        }
    
       
        void turnOnSound() {
            sound = true;
        }
    
        void turnOffSound() {
            sound = false;
        }
    
        void turnOnMusic() {
            music = true;
        }
    
        void turnOffMusic() {
            music = false;
        }
    };
    
 //hey dont touch these thing for now it will be gotten from other header file 
enum STATES{
    LOBBY,
    PLAY,
    EXIT,
    MODE,
    THEME,
    SETTINGS
};
STATES gameSTATE = LOBBY; 

Vector2 getScreenCenter(Vector2 size) {
    Vector2 coords = { (GetScreenWidth() - size.x) / 2, (GetScreenHeight() - size.y) / 2 };
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
        imgTexture = { 0 };
        coords = { 0, 0 };
    }

    comp(Vector2 coords, Texture2D texture) : coords(coords), imgTexture(texture) {
        this->coords.x *= SCALE;
        this->coords.y *= SCALE;
    }

    inline Vector2 getSize() const {
        return Vector2{ float(imgTexture.width * SCALE), float(imgTexture.height * SCALE) };
    }

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
        if(c!='c') return;
        Vector2 COORDS = {
            float((GetScreenWidth() - (imgTexture.width * SCALE)) / 2),
            float((GetScreenHeight() - (imgTexture.height * SCALE)) / 2)
        };
        DrawTextureEx(imgTexture, COORDS, 0, SCALE, WHITE);
    }

    ~comp() {
        if (imgTexture.id != 0) {
            UnloadTexture(imgTexture);
        }
    }
};

class Button : public comp {
protected:
    Texture2D pressedImgTexture = { 0 };
    Texture2D unpressedImgTexture = { 0 };

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
        pressedImgTexture.width =buttonWidth;
        imgTexture = unpressedImgTexture;
         

        UnloadImage(img1);
        UnloadImage(img2);
    }

    void hover() {
        Rectangle bounds = {
            coords.x,
            coords.y,
            float(imgTexture.width * SCALE),
            float(imgTexture.height * SCALE)
        };

        if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
            imgTexture = pressedImgTexture;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
          
        } else {
            imgTexture = unpressedImgTexture;
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        
    }
    virtual void click() = 0;
    void handleEvent(){
        draw();
        hover();
        click();

    }

    ~Button() {
        if (pressedImgTexture.id != 0) UnloadTexture(pressedImgTexture);
        if (unpressedImgTexture.id != 0) UnloadTexture(unpressedImgTexture);
    }
};
class playButton : public Button{
    public:
    void click(){
    gameSTATE = PLAY;
    }
};
class selectMode: public Button{
    public:
    void click(){
    gameSTATE = MODE;
    }
};
class selectTheme: public Button{

    public:
    void click(){
    gameSTATE = THEME;
    }


};
class settings: public Button{

    public:

    void click(){
    gameSTATE = SETTINGS;
    }

};
class exit: public Button{

    public:
    void click(){
    gameSTATE = EXIT;
    }


};

class DialogueBox  : public comp{

private:
    vector<Button*>Buttons;
    void calcPositions(){

        float xoffset = 21.3;
        float yoffset = 67.0;
        Vector2 SIZE = {buttonHeight,buttonWidth};
        Vector2 COORDS;
        for (int i = 0;i<Buttons.size();i++)
        {
            
            Buttons[i].setSize(SIZE);
            COORDS = {xoffset += (Buttons[i].getSize().x) ,yoffset  +  (Buttons[i].getSize().y)};
            Buttons[i].setCoords(COORDS);
        }
    
    
    }
public:
DialogueBox() : comp(){
    calcPositions();
}
void setButton(Button BTN){
Buttons.push_back(BTN);
}

void draw(){
        comp::draw('c');
        
        for (int i = 0;i<Buttons.size();i++)
        {
          Buttons[i]->draw();
        }
        

}






};


class Player{

    private:
    string playerName;
    gameMode Mode;
    gameTHEME Theme;
    settings playerSettings;
    // Paddle class
    // Ball class
    };



int main() {
    InitWindow(1920, 1080, "LOBBY SCREEN");
    SetTargetFPS(60);
    calcScale();
    comp bg;
  

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
