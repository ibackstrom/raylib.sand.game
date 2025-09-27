#include "raylib.h"

int main() {
    InitWindow(800, 450, "Sand Game Test");
    SetTargetFPS(60);
    TraceLog(LOG_INFO, "Window initialized"); // Debug log
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello, Sand!", 10, 10, 20, BLACK); // Test text
        EndDrawing();
    }
    CloseWindow();
    return 0;
}