#ifndef __MASTERMIND_H__
#define __MASTERMIND_H__

#include <ctime>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shaders.h"
#include "objects.h"

namespace mastermind
{

class Engine
{
    enum State
    {
        PLAYING,
        LOST,
        WON
    }; // enum State

    enum BoardState
    {
        EMPTY,
        COLOR_1,
        COLOR_2,
        COLOR_3,
        COLOR_4,
        COLOR_5,
        COLOR_6
    }; // enum BoardState

    State state;
    BoardState answer[OBJECTS::BOARD_COLS];
    BoardState board[OBJECTS::BOARD_ROWS][OBJECTS::BOARD_COLS];
    bool clues[OBJECTS::BOARD_ROWS][OBJECTS::BOARD_COLS];

    struct Current
    {
        int row;
        int col;
    } current;

    struct GL
    {
        GLuint VAI; // Vertex Array ID
        GLuint borders;
        GLuint circles;
        GLuint colors;
        GLuint _final;
        GLuint shaders;
    } gl;

    void colorSmallCircle(int row, int col, int color);
    void colorBigCircle(int row, int col, int color);

    public:
        Engine(void);
        ~Engine(void);

        void init(void);
        void draw(void);

        bool commit(void);
        bool change(int key);
        bool select(int key);

}; // class Board

inline
void Engine::colorSmallCircle(int row, int col, int color)
{
    GLfloat Circle[OBJECTS::CIRCLE_POINTS * 3] = {};
    for(int p = 0; p < OBJECTS::CIRCLE_POINTS * 3; p += 3)
    {
        Circle[p]   = OBJECTS::COLOR[color][0];
        Circle[p+1] = OBJECTS::COLOR[color][1];
        Circle[p+2] = OBJECTS::COLOR[color][2];
    }

    glBindBuffer(GL_ARRAY_BUFFER, gl.colors);
    glBufferSubData(GL_ARRAY_BUFFER, OBJECTS::SmallCircleIndex(row, col) * 3 * sizeof(GLfloat), sizeof(Circle), &Circle);
}

inline
void Engine::colorBigCircle(int row, int col, int color)
{
    GLfloat Circle[OBJECTS::CIRCLE_POINTS * 3] = {};
    for(int p = 0; p < OBJECTS::CIRCLE_POINTS * 3; p += 3)
    {
        Circle[p]   = OBJECTS::COLOR[color][0];
        Circle[p+1] = OBJECTS::COLOR[color][1];
        Circle[p+2] = OBJECTS::COLOR[color][2];
    }

    glBindBuffer(GL_ARRAY_BUFFER, gl.colors);
    glBufferSubData(GL_ARRAY_BUFFER, OBJECTS::BigCircleIndex(row, col) * 3 * sizeof(GLfloat), sizeof(Circle), &Circle);
}

inline
Engine::Engine(void)
:state(PLAYING)
,answer()
,board()
,clues()
,current()
,gl()
{
    srand(time(nullptr));
    for(int a = 0; a < OBJECTS::BOARD_COLS; ++ a)
        switch(rand() % 6)
        {
            case 0:
                answer[a] = BoardState::COLOR_1;
                break;

            case 1:
                answer[a] = BoardState::COLOR_2;
                break;

            case 2:
                answer[a] = BoardState::COLOR_3;
                break;

            case 3:
                answer[a] = BoardState::COLOR_4;
                break;

            case 4:
                answer[a] = BoardState::COLOR_5;
                break;

            case 5:
                answer[a] = BoardState::COLOR_6;
                break;
        }
}

inline
Engine::~Engine(void)
{
    if(gl.shaders)
        glDeleteProgram(gl.shaders);

    if(gl.borders)
        glDeleteBuffers(1, &gl.borders);

    if(gl.circles)
        glDeleteBuffers(1, &gl.circles);

    if(gl.colors)
         glDeleteBuffers(1, &gl.colors);

    if(gl._final)
        glDeleteBuffers(1, &gl._final);

    if(gl.VAI)
        glDeleteVertexArrays(1, &gl.VAI);
}

inline
void Engine::init(void)
{
    glGenVertexArrays(1, &gl.VAI);
    glBindVertexArray(gl.VAI);

    // Board border
    glGenBuffers(1, &gl.borders);
    glBindBuffer(GL_ARRAY_BUFFER, gl.borders);
    glBufferData(GL_ARRAY_BUFFER, sizeof(OBJECTS::Border), OBJECTS::Border, GL_STATIC_DRAW);

    // Board circles
    glGenBuffers(1, &gl.circles);
    glBindBuffer(GL_ARRAY_BUFFER, gl.circles);
    glBufferData(GL_ARRAY_BUFFER, sizeof(OBJECTS::Circles), OBJECTS::Circles, GL_STATIC_DRAW);

    // Board colors
    glGenBuffers(1, &gl.colors);
    glBindBuffer(GL_ARRAY_BUFFER, gl.colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(OBJECTS::Colors), OBJECTS::Colors, GL_STATIC_DRAW);

    // Load shaders
    gl.shaders = LoadShaders("simple.vertexshader", "simple.fragmentshader");

    // Final buffer
    glGenBuffers(1, &gl._final);
    glBindBuffer(GL_ARRAY_BUFFER, gl._final);
    glBufferData(GL_ARRAY_BUFFER, sizeof(OBJECTS::Final), OBJECTS::Final, GL_STATIC_DRAW);
}

inline
void Engine::draw(void)
{
    // Draw board
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, gl.borders);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_LINE_STRIP, 0, OBJECTS::BorderPoints);
    glDisableVertexAttribArray(0);

    // Draw circles
    glUseProgram(gl.shaders);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, gl.circles);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, gl.colors);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_FAN, OBJECTS::HighlightCircleIndex(current.row, current.col), OBJECTS::CIRCLE_POINTS);
    for(int r = 0; r < OBJECTS::BOARD_ROWS; ++ r)
        for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
            if(board[r][c] != BoardState::EMPTY)
                glDrawArrays(GL_TRIANGLE_FAN, OBJECTS::BigCircleIndex(r, c), OBJECTS::CIRCLE_POINTS);

    for(int r = 0; r < OBJECTS::BOARD_ROWS; ++ r)
        for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
            if(clues[r][c])
                glDrawArrays(GL_TRIANGLE_FAN, OBJECTS::SmallCircleIndex(r, c), OBJECTS::CIRCLE_POINTS);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glUseProgram(0);

    if(state == LOST)
    {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, gl._final);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLE_FAN,   OBJECTS::FACE_INDEX,            OBJECTS::CIRCLE_POINTS);
        glUseProgram(gl.shaders);
        glDrawArrays(GL_TRIANGLE_FAN,   OBJECTS::FACE_LEFT_EYE_INDEX,   OBJECTS::CIRCLE_POINTS);
        glDrawArrays(GL_TRIANGLE_FAN,   OBJECTS::FACE_RIGHT_EYE_INDEX,  OBJECTS::CIRCLE_POINTS);
        glDrawArrays(GL_LINE_STRIP,     OBJECTS::FACE_SAD_INDEX,        OBJECTS::FACE_SMILE_POINTS);
        glEnableVertexAttribArray(0);
    }

    if(state == WON)
    {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, gl._final);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLE_FAN,   OBJECTS::FACE_INDEX,            OBJECTS::CIRCLE_POINTS);
        glUseProgram(gl.shaders);
        glDrawArrays(GL_TRIANGLE_FAN,   OBJECTS::FACE_LEFT_EYE_INDEX,   OBJECTS::CIRCLE_POINTS);
        glDrawArrays(GL_TRIANGLE_FAN,   OBJECTS::FACE_RIGHT_EYE_INDEX,  OBJECTS::CIRCLE_POINTS);
        glDrawArrays(GL_LINE_STRIP,     OBJECTS::FACE_HAPPY_INDEX,      OBJECTS::FACE_SMILE_POINTS);
        glEnableVertexAttribArray(0);
    }

    glUseProgram(0);
}

inline
bool Engine::commit(void)
{
    if(state != PLAYING)
        return false;

    bool valid = true,
         finished = true;
    for(int c = 0; c < OBJECTS::BOARD_COLS && valid; ++ c)
    {
        valid = board[current.row][c] != BoardState::EMPTY;
        finished &= board[current.row][c] == answer[c];
    }

    if(!valid)
        return false;

    if(finished)
    {
        state = WON;
        return true;
    }

    int clue = 0;
    bool found[OBJECTS::BOARD_COLS] = {};
    for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
        if(board[current.row][c] == answer[c])
        {
            found[c] = true;
            clues[current.row][clue] = true;
            colorSmallCircle(current.row, clue ++, 7);
        }

    for(int c = 0; c < OBJECTS::BOARD_COLS; ++ c)
        if(!found[c])
        {
            bool color = false;
            for(int c2 = 0; c2 < OBJECTS::BOARD_COLS && !color; ++ c2)
                color = board[current.row][c] == answer[c2] && !found[c2];

            if(!color)
                continue;

            found[c] = true;
            clues[current.row][clue] = true;
            colorSmallCircle(current.row, clue ++, 0);
        }

    if(current.row == OBJECTS::BOARD_ROWS - 1)
    {
        state = LOST;
        return true;
    }

    current.row += 1;
    return true;
}

inline
bool Engine::change(int key)
{
    if(state != PLAYING)
        return false;

    switch(key)
    {
        case GLFW_KEY_1:
            board[current.row][current.col] = BoardState::COLOR_1;
            break;

        case GLFW_KEY_2:
            board[current.row][current.col] = BoardState::COLOR_2;
            break;

        case GLFW_KEY_3:
            board[current.row][current.col] = BoardState::COLOR_3;
            break;

        case GLFW_KEY_4:
            board[current.row][current.col] = BoardState::COLOR_4;
            break;

        case GLFW_KEY_5:
            board[current.row][current.col] = BoardState::COLOR_5;
            break;

        case GLFW_KEY_6:
            board[current.row][current.col] = BoardState::COLOR_6;
            break;

        case GLFW_KEY_UP:
            board[current.row][current.col] = (BoardState) (1 + board[current.row][current.col] % 6);
            break;

        case GLFW_KEY_DOWN:
            board[current.row][current.col] = (BoardState) (1 + (board[current.row][current.col] + 4) % 6);
            break;

        default:
            return false;
    }

    colorBigCircle(current.row, current.col, board[current.row][current.col]);
    return true;
}

inline
bool Engine::select(int key)
{
    if(state != PLAYING)
        return false;

    switch(key)
    {
        case GLFW_KEY_LEFT:
            current.col = (current.col + 3) % 4;
            break;

        case GLFW_KEY_RIGHT:
            current.col = (current.col + 1) % 4;
            break;

        default:
            return false;
    }

    return true;
}

}; // namespace mastermind

#endif // __MASTERMIND_H__
