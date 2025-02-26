#include <iostream>
#include "raylib.h"
using namespace std;

int scoreLeft=0,scoreRight=0;
const int screenWidth=800,screenHeight=600;


class Ball {
    float x;
    float y;
    int speedX=5;
    int speedY=5;
    int radius;
    Color ballColor;
    public:
        Ball(float Px,float Py,int Bradius,Color ballclr) {
            x=Px;
            y=Py;
            radius=Bradius;
            ballColor=ballclr;
        }
        void Draw() {
            DrawCircle(x,y,radius,ballColor);
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
};

class paddle {
    int x;
    int y;
    int width;
    int height;
    int speed;
    Color clr;
    public:
        paddle(int px,int py,int width,int height,Color color,int speed) {
            x=px;
            y=py;
            this->width=width;
            this->height=height;
            clr=color;
            this->speed=speed;
        }
        Rectangle getRec() {
            Rectangle rec={float(x),float(y),float(width),float(height)};
            return rec;
        }
        void Draw() {
            DrawRectangle(x,y,width,height,clr);
        }
        void update(string side) {
            if(side == "left") {
                if(IsKeyDown(KEY_W) && y-speed <=480 && y-speed >=0) y -= speed;
                if(IsKeyDown(KEY_S) && y+speed <=480 && y+speed >=0) y += speed;
            }
            else {
                if(IsKeyDown(KEY_UP) && y-speed <=480 && y-speed >=0) y -= speed;
                if(IsKeyDown(KEY_DOWN) && y+speed <=480 && y+speed >=0) y += speed;
            }
        }
};

int main() {
    InitWindow(800,600,"Pong Game");
    SetTargetFPS(60);
    Ball ball(screenWidth/2,screenHeight/2,20,WHITE);
    paddle left(10,screenHeight/2 - 60,25,120,WHITE,7);
    paddle right(screenWidth-35,screenHeight/2 - 60,25,120,WHITE,7);
    Rectangle boundaries={0,0,800,600};
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({50, 168, 82,255});
        ball.Update(boundaries,left.getRec(),right.getRec());
        left.update("left");
        right.update("right");
        
        DrawText(TextFormat("%d", scoreLeft), screenWidth/4, 20, 40, BLACK);
        DrawText(TextFormat("%d", scoreRight), 3*screenWidth/4, 20, 40, BLACK);

        DrawRectangleLinesEx(boundaries,4,YELLOW);
        DrawLine(screenWidth/2,0,screenWidth/2,screenHeight,WHITE);
        DrawCircleLines(screenWidth/2,screenHeight/2,100,WHITE);
        ball.Draw();
        left.Draw();
        right.Draw();
        EndDrawing();
    }
    CloseWindow();
}
