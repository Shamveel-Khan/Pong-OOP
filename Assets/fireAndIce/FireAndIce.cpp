#include <iostream>
#include "raylib.h"
using namespace std;

int scoreLeft=0,scoreRight=0;
const int screenWidth=800,screenHeight=800;


class Ball {
    float x;
    float y;
    int speedX=5;
    int speedY=5;
    int radius;
    Color ballColor;
    Texture2D texture;
    public:
        Ball(float Px,float Py,int Bradius,Color ballclr) {
            x=Px;
            y=Py;
            radius=Bradius;
            ballColor=ballclr;
            Image ball = LoadImage("ball.png");
            ImageResize(&ball,radius*2,radius*2);

            texture = LoadTextureFromImage(ball);
        }
       
           
            void Draw() {
                DrawTexture(texture, x - radius, y - radius, WHITE);  
            }
        
        void Update(Rectangle rect,Rectangle leftPaddle, Rectangle rightPaddle) {
            if (x - radius <= rect.x || x + radius >= rect.x + rect.width) {
                speedX *= -1;
                if(x > screenWidth/2) {
                    scoreLeft++;
                }
                else {
                    scoreRight++;
                }  
            }
            if (y - radius <= rect.y || y + radius >= rect.y + rect.height) {
                speedY *= -1;
            }
            if (CheckCollisionCircleRec({x, y}, radius, leftPaddle) ||
            CheckCollisionCircleRec({x, y}, radius, rightPaddle)) {
                speedX *= -1;  
            }
            x += speedX;
            y += speedY;
        }

        ~Ball() {
            UnloadTexture(texture);
        }
};

class paddle {
    int x;
    int y;
    int width;
    int height;
    int speed;
    Color clr;
    Texture2D texture;
    public:
        paddle(int px,int py,int width,int height,Color color,int speed) {
            x=px;
            y=py;
            this->width=width;
            this->height=height;
            clr=color;
            this->speed=speed;

            Image paddle = LoadImage("paddle1.png");
            ImageResize(&paddle,width,height);

            texture = LoadTextureFromImage(paddle);
        }
        Rectangle getRec() {
            Rectangle rec={float(x),float(y),float(width),float(height)};
            return rec;
        }
        void Draw() {
            // DrawRectangle(x,y,width,height,clr);
            DrawTexture(texture,x,y,WHITE);
        }
        void update(string side) {
            if(side == "left") {
                if(IsKeyDown(KEY_W) && y-speed <=680 && y-speed >=0) y -= speed;
                if(IsKeyDown(KEY_S) && y+speed <=680 && y+speed >=0) y += speed;
            }
            else {
                if(IsKeyDown(KEY_UP) && y-speed <=680 && y-speed >=0) y -= speed;
                if(IsKeyDown(KEY_DOWN) && y+speed <=680 && y+speed >=0) y += speed;
            }
        }
};

class Button{
    protected:
    Texture2D texture;
    Vector2 xy_plane;
    Rectangle location;
    public:
    Button(const char* path, Vector2 position, int width, int height){
       Image gearImage = LoadImage(path);
        xy_plane = position;
        ImageResize(&gearImage,width,height);
        texture = LoadTextureFromImage(gearImage);
        UnloadImage(gearImage);

        location = {position.x,position.y,float(width),float(height)};

    }

    void draw(){
        DrawTextureV(texture,xy_plane,WHITE);
    }
    
    bool isClicked(){
        return CheckCollisionPointRec(GetMousePosition(), location) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }
    ~Button(){
        UnloadTexture(texture);
    }
};
class GearButton : public Button{
    public:
    GearButton(const char* path, Vector2 position, int width, int height) : Button(path,position,width,height){}


};

class PauseButton : public Button{

    public:
    PauseButton(const char* path, Vector2 position, int width, int height) : Button(path,position,width,height){
  
    }

    ~PauseButton(){
        UnloadTexture(texture);
    }

};

class ResumeButton : public Button{
    public:
    ResumeButton(const char* path, Vector2 position, int width, int height) : Button(path,position,width,height){
  
    }

    ~ResumeButton(){
        UnloadTexture(texture);
    }

};

class ExitButton : public Button{
    public:
    ExitButton(const char* path, Vector2 position, int width, int height) : Button(path,position,width,height){
  
    }

    ~ExitButton(){
        UnloadTexture(texture);
    }

};



int main () {
   
    InitWindow(800,800,"Fire And Ice");
    SetTargetFPS(60);
    Image spaceGalaxyBackground = LoadImage("fireAndIcebackground.png");
    ImageResize(&spaceGalaxyBackground,800,800);

    Texture2D img = LoadTextureFromImage(spaceGalaxyBackground);

    GearButton gear("gear.png",{500,25},80,80);
    PauseButton pause("pause.png",{200,25},80,80);
    ResumeButton resume("resume.png",{300,300},200,60);
    ExitButton exit("exit.png",{300,380},200,60);

    Ball ball(screenWidth/2,screenHeight/2,25,WHITE);
    paddle left(10,screenHeight/2 - 60,40,150,WHITE,7);
    paddle right(screenWidth-35,screenHeight/2 - 60,40,150,WHITE,7);
    Rectangle boundaries= {0,0,800,800};
    
    bool toggleGear = false;
    bool togglePause = false;
    bool showGear = true;
    bool showPause = true;
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        ball.Update(boundaries,left.getRec(),right.getRec());
        left.update("left");
        right.update("right");

        DrawTexture(img,0,0,WHITE);
        DrawText(TextFormat("%i",scoreLeft),90,350,120,(Color){255, 140, 0, 255});
        DrawText(TextFormat("%i",scoreRight),657,350,120,(Color){150, 255, 255, 255});
        DrawLineEx({400,0},{400,800},5.0f,(Color){150,255,255,255});
       
        if(showGear){
            gear.draw();
        }
       if(showPause){
        pause.draw();
       }
       

        if(gear.isClicked()){
           toggleGear = true;
        }
        if(toggleGear){
            resume.draw();
            exit.draw();
            showPause = false;
            if(resume.isClicked()){
                toggleGear = false;
                showPause = true;
            }

            if(exit.isClicked()){
                CloseWindow();
                return 0;
            }
         }

         if(pause.isClicked()){
            togglePause = true;
         }

         if(togglePause){
            DrawText("Game Is Paused",150,400,60,RAYWHITE);
            DrawText("PRESS SPACE TO CONTINUE",210,460,24,RAYWHITE);
            showGear = false;
            if(IsKeyDown(KEY_SPACE)){
                togglePause = false;
                showGear = true;
            }
         }
        
            ball.Draw();
            left.Draw();
            right.Draw();   

        EndDrawing();
    }
    UnloadTexture(img);
    CloseWindow();
    
   
}
