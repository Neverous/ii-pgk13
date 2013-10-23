#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "objects.h"

inline float min(float a, float b) { return a<b?a:b; }

inline
static void writeCircle(int points, GLfloat cx, GLfloat cy, GLfloat r)
{
    int triangles = points - 3;
    printf("%8.5ff, %8.5ff, %8.5ff,\n", cx, cy, 0.00f);
    for(int t = 0; t <= triangles + 1; ++ t)
        printf("%8.5ff, %8.5ff, %8.5ff,\n", cx + r * cos(t * M_PI * 2.00f / triangles), cy + r * sin(t * M_PI * 2.00f / triangles), 0.00f);
}

int main(void)
{
    // Big Circles
    for(int r = 0; r < OBJECTS::BOARD_ROWS; ++ r)
        for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
            writeCircle(OBJECTS::CIRCLE_POINTS, -1.00f + (0.50f + c) * 2.00f / (OBJECTS::BOARD_COLS + 1), 1.00f - (0.50f + r) * 2.00f / OBJECTS::BOARD_ROWS, min(1.00f / (OBJECTS::BOARD_COLS + 1), 1.00f / OBJECTS::BOARD_ROWS));

    // Small Circles
    for(int r = 0; r < 8; ++ r)
        for(int c = 0; c < 4; ++ c)
            writeCircle(OBJECTS::CIRCLE_POINTS, 1.00f - 2.00f / (OBJECTS::BOARD_COLS + 1) + (0.50f + (c&1)) * 1.00f / (OBJECTS::BOARD_COLS + 1), 1.00f - (0.25f + r) * 2.00f / OBJECTS::BOARD_ROWS - (c>1) * 1.00f / OBJECTS::BOARD_ROWS, min(0.50f / (OBJECTS::BOARD_COLS + 1), 0.50f / OBJECTS::BOARD_ROWS));

    // Highlight Circles
    for(int r = 0; r < OBJECTS::BOARD_ROWS; ++ r)
        for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
            writeCircle(OBJECTS::CIRCLE_POINTS, -1.00f + (0.50f + c) * 2.00f / (OBJECTS::BOARD_COLS + 1), 1.00f - (0.50f + r) * 2.00f / OBJECTS::BOARD_ROWS, min(1.01f / (OBJECTS::BOARD_COLS + 1), 1.01f / OBJECTS::BOARD_ROWS));

    /*
    // Colors
    for(int t = 0; t < 3; ++ t)
    for(int r = 0; r < OBJECTS::BOARD_ROWS; ++ r)
        for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
            for(int p = 0; p < OBJECTS::CIRCLE_POINTS; ++ p)
                puts(" 1.00f,  1.00f,  1.00f,");
    */


    // Final
    /*
    writeCircle(OBJECTS::CIRCLE_POINTS, 0.00f, 0.00f, 0.75f);
    writeCircle(OBJECTS::CIRCLE_POINTS, -0.50f, 0.30f, 0.15f);
    writeCircle(OBJECTS::CIRCLE_POINTS, 0.50f, 0.30f, 0.15f);

    puts(" -0.50f, -0.10f,  0.00f,");
    puts(" -0.30f, -0.20f,  0.00f,");
    puts("  0.30f, -0.20f,  0.00f,");
    puts("  0.50f, -0.10f,  0.00f,");

    puts(" -0.50f, -0.30f,  0.00f,");
    puts(" -0.30f, -0.20f,  0.00f,");
    puts("  0.30f, -0.20f,  0.00f,");
    puts("  0.50f, -0.30f,  0.00f,");

    */
    return 0;
}
