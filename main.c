#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define GRID_WIDTH 200
#define GRID_HEIGHT 150
#define CELL_SIZE 5
#define WINDOW_WIDTH (GRID_WIDTH * CELL_SIZE)
#define WINDOW_HEIGHT (GRID_HEIGHT * CELL_SIZE)

typedef enum {
    EMPTY = 0,
    SAND = 1,
    WATER = 2,
    BOX = 3
} CellType;

typedef struct {
    float x, y;
} Vector2f;

typedef struct {
    CellType cells[GRID_WIDTH][GRID_HEIGHT];
    Vector2f velocity[GRID_WIDTH][GRID_HEIGHT]; // Velocity for water cells
} Grid;

void InitGrid(Grid* grid) {
    memset(grid->cells, EMPTY, sizeof(grid->cells));
    memset(grid->velocity, 0, sizeof(grid->velocity));

    // Initialize two boxes higher above the ground
    for (int x = 50; x <= 69; x++) {
        for (int y = 120; y <= 129; y++) {
            grid->cells[x][y] = BOX;
            grid->velocity[x][y].x = 0;
            grid->velocity[x][y].y = 0;
        }
    }
    for (int x = 130; x <= 149; x++) {
        for (int y = 120; y <= 129; y++) {
            grid->cells[x][y] = BOX;
            grid->velocity[x][y].x = 0;
            grid->velocity[x][y].y = 0;
        }
    }
}

void AddSand(Grid* grid, int x, int y, int radius) {
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            int px = x + i;
            int py = y + j;
            if (px >= 0 && px < GRID_WIDTH && py >= 0 && py < GRID_HEIGHT) {
                float dist = sqrtf(i * i + j * j);
                if (dist <= radius && grid->cells[px][py] != BOX) {
                    grid->cells[px][py] = SAND;
                    grid->velocity[px][py].x = 0;
                    grid->velocity[px][py].y = 0;
                }
            }
        }
    }
}

void AddWater(Grid* grid, int x, int y, int radius) {
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            int px = x + i;
            int py = y + j;
            if (px >= 0 && px < GRID_WIDTH && py >= 0 && py < GRID_HEIGHT) {
                float dist = sqrtf(i * i + j * j);
                if (dist <= radius && grid->cells[px][py] != SAND && grid->cells[px][py] != BOX) {
                    grid->cells[px][py] = WATER;
                    grid->velocity[px][py].x = 0;
                    grid->velocity[px][py].y = 1.0f; // Initial downward velocity
                }
            }
        }
    }
}

void UpdateGrid(Grid* grid) {
    // Update sand (bottom to top)
    for (int y = GRID_HEIGHT - 2; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid->cells[x][y] == SAND) {
                if (y + 1 < GRID_HEIGHT && grid->cells[x][y + 1] != SAND && grid->cells[x][y + 1] != BOX) {
                    grid->cells[x][y] = EMPTY;
                    grid->cells[x][y + 1] = SAND;
                    grid->velocity[x][y + 1] = grid->velocity[x][y];
                    grid->velocity[x][y].x = 0;
                    grid->velocity[x][y].y = 0;
                }
                else if (y + 1 < GRID_HEIGHT) {
                    int dir = GetRandomValue(0, 1) ? 1 : -1;
                    int newX = x + dir;
                    if (newX >= 0 && newX < GRID_WIDTH && grid->cells[newX][y + 1] != SAND && grid->cells[newX][y + 1] != BOX) {
                        grid->cells[x][y] = EMPTY;
                        grid->cells[newX][y + 1] = SAND;
                        grid->velocity[newX][y + 1] = grid->velocity[x][y];
                        grid->velocity[x][y].x = 0;
                        grid->velocity[x][y].y = 0;
                    }
                    else {
                        newX = x - dir;
                        if (newX >= 0 && newX < GRID_WIDTH && grid->cells[newX][y + 1] != SAND && grid->cells[newX][y + 1] != BOX) {
                            grid->cells[x][y] = EMPTY;
                            grid->cells[newX][y + 1] = SAND;
                            grid->velocity[newX][y + 1] = grid->velocity[x][y];
                            grid->velocity[x][y].x = 0;
                            grid->velocity[x][y].y = 0;
                        }
                    }
                }
            }
        }
    }

    // Update water (bottom to top, FLIP-like behavior)
    Vector2f* newVelocity = (Vector2f*)malloc(GRID_WIDTH * GRID_HEIGHT * sizeof(Vector2f));
    memcpy(newVelocity, grid->velocity, GRID_WIDTH * GRID_HEIGHT * sizeof(Vector2f));
    for (int y = GRID_HEIGHT - 2; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid->cells[x][y] == WATER) {
                newVelocity[x + y * GRID_WIDTH].y += 0.1f; // Gravity

                if (y + 1 < GRID_HEIGHT && grid->cells[x][y + 1] == EMPTY) {
                    grid->cells[x][y] = EMPTY;
                    grid->cells[x][y + 1] = WATER;
                    newVelocity[x + (y + 1) * GRID_WIDTH] = newVelocity[x + y * GRID_WIDTH];
                    newVelocity[x + y * GRID_WIDTH].x = 0;
                    newVelocity[x + y * GRID_WIDTH].y = 0;
                }
                else if (y + 1 < GRID_HEIGHT && grid->cells[x][y + 1] != EMPTY && grid->cells[x][y + 1] != BOX) {
                    int directions[2] = {1, -1};
                    for (int i = 0; i < 2; i++) {
                        int newX = x + directions[i];
                        if (newX >= 0 && newX < GRID_WIDTH && grid->cells[newX][y] == EMPTY) {
                            grid->cells[x][y] = EMPTY;
                            grid->cells[newX][y] = WATER;
                            newVelocity[newX + y * GRID_WIDTH] = newVelocity[x + y * GRID_WIDTH];
                            newVelocity[newX + y * GRID_WIDTH].x += directions[i] * 0.5f;
                            newVelocity[x + y * GRID_WIDTH].x = 0;
                            newVelocity[x + y * GRID_WIDTH].y = 0;
                            break;
                        }
                    }
                }
                else if (y + 1 < GRID_HEIGHT) {
                    int dir = GetRandomValue(0, 1) ? 1 : -1;
                    int newX = x + dir;
                    if (newX >= 0 && newX < GRID_WIDTH && grid->cells[newX][y + 1] == EMPTY) {
                        grid->cells[x][y] = EMPTY;
                        grid->cells[newX][y + 1] = WATER;
                        newVelocity[newX + (y + 1) * GRID_WIDTH] = newVelocity[x + y * GRID_WIDTH];
                        newVelocity[x + y * GRID_WIDTH].x = 0;
                        newVelocity[x + y * GRID_WIDTH].y = 0;
                    }
                    else {
                        newX = x - dir;
                        if (newX >= 0 && newX < GRID_WIDTH && grid->cells[newX][y + 1] == EMPTY) {
                            grid->cells[x][y] = EMPTY;
                            grid->cells[newX][y + 1] = WATER;
                            newVelocity[newX + (y + 1) * GRID_WIDTH] = newVelocity[x + y * GRID_WIDTH];
                            newVelocity[x + y * GRID_WIDTH].x = 0;
                            newVelocity[x + y * GRID_WIDTH].y = 0;
                        }
                    }
                }
            }
        }
    }

    // Pressure projection with bounds checking
    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        for (int y = 1; y < GRID_HEIGHT - 1; y++) {
            if (grid->cells[x][y] == WATER) {
                float avgX = 0.0f, avgY = 0.0f;
                int count = 0;
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                            avgX += newVelocity[nx + ny * GRID_WIDTH].x;
                            avgY += newVelocity[nx + ny * GRID_WIDTH].y;
                            count++;
                        }
                    }
                }
                if (count > 0) {
                    avgX /= count;
                    avgY /= count;
                    newVelocity[x + y * GRID_WIDTH].x = newVelocity[x + y * GRID_WIDTH].x * 0.5f + avgX * 0.5f;
                    newVelocity[x + y * GRID_WIDTH].y = newVelocity[x + y * GRID_WIDTH].y * 0.5f + avgY * 0.5f;
                    if (fabsf(newVelocity[x + y * GRID_WIDTH].x) < 0.01f) newVelocity[x + y * GRID_WIDTH].x = 0;
                    if (fabsf(newVelocity[x + y * GRID_WIDTH].y) < 0.01f) newVelocity[x + y * GRID_WIDTH].y = 0;
                }
            }
        }
    }
    memcpy(grid->velocity, newVelocity, GRID_WIDTH * GRID_HEIGHT * sizeof(Vector2f));
    free(newVelocity); // Free the heap-allocated array
}

void DrawSimulationGrid(Grid* grid) {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (grid->cells[x][y] == SAND) {
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, (Color){255, 204, 102, 255});
            }
            else if (grid->cells[x][y] == WATER) {
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, (Color){0, 105, 255, 200});
            }
            else if (grid->cells[x][y] == BOX) {
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, (Color){255, 255, 255, 255});
            }
        }
    }
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sand and Water Simulation");
    SetTargetFPS(60);

    Grid grid;
    InitGrid(&grid);
    int spawnRadius = 1;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
            spawnRadius = (spawnRadius < 5) ? spawnRadius + 1 : spawnRadius;
        }
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            spawnRadius = (spawnRadius > 1) ? spawnRadius - 1 : spawnRadius;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int gridX = (int)(mousePos.x / CELL_SIZE);
            int gridY = (int)(mousePos.y / CELL_SIZE);
            AddSand(&grid, gridX, gridY, spawnRadius);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 mousePos = GetMousePosition();
            int gridX = (int)(mousePos.x / CELL_SIZE);
            int gridY = (int)(mousePos.y / CELL_SIZE);
            AddWater(&grid, gridX, gridY, spawnRadius);
        }

        UpdateGrid(&grid);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawSimulationGrid(&grid);
        DrawRectangleLines(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, WHITE);

        DrawText("IGOR YAKUNIN", 10, 40, 30, WHITE);
        DrawText("Left mouse: Produce sand", 10, 80, 20, WHITE);
        DrawText("Right mouse: Produce water", 10, 100, 20, WHITE);

        const char* plusText = "+: Increase spawn amount";
        const char* minusText = "-: Decrease spawn amount";
        int plusWidth = MeasureText(plusText, 20);
        int minusWidth = MeasureText(minusText, 20);
        DrawText(plusText, WINDOW_WIDTH - plusWidth - 10, 80, 20, WHITE);
        DrawText(minusText, WINDOW_WIDTH - minusWidth - 10, 100, 20, WHITE);

        char radiusText[32];
        snprintf(radiusText, sizeof(radiusText), "Spawn radius: %d", spawnRadius);
        DrawText(radiusText, 10, 130, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}