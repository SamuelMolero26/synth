#include <stdio.h>
#include "raylib.h"

void main(){
    const int screen_width = 1024;
    const int screen_height = 768;
    initWindow(screen_width, screen_height, "Synth");
    SetTargetFPS(60);
    while(!WindownShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Synth", 100, 100, 20, GREEN);
        EndDrawing();

    }
    CloseWindown(); 
    
}