#ifndef __MASTERMIND_H__
#define __MASTERMIND_H__

namespace mastermind
{
class Engine;

class Board
{
    friend class Engine;
    enum State
    {
        EMPTY,
        COLOR_1,
        COLOR_2,
        COLOR_3,
        COLOR_4,
        COLOR_5,
        COLOR_6
    }; // enum State

    State board[32];

    public:
        Board(void) {};
        void draw(void *window) {};
}; // class Board

class Guess
{
    friend class Engine;
    public:
        Guess(void) {};
        bool commit(void) {};
        bool change(int key) {};
        bool select(int key) {};
        void draw(void *window) {};
}; // class Guess

class Engine
{
    public:
        Board board;
        Guess guess;

        Engine(void) {};
        void draw(void *window) {};
}; // class Engine

}; // namespace mastermind

#endif // __MASTERMIND_H__
