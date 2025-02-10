#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <iostream>
#include <string>
#include <random>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FPS = 10;
const int frameDelay = 1000 / FPS;
const int speed = 1;
const int pixel_size = 32;

enum class GameState {
    MENU,
    GAME,
    GAME_OVER
};

struct SnakeSegment {
    int x, y;
};

class SnakeGame {
public:
    SnakeGame();
    ~SnakeGame();

    Uint32 _frameStart;

    void init();
    void initSnake(int x, int y, int length);
    void setApplePosition(int x, int y);
    bool loadTextures();
    void handleEvents();
    void update();
    bool checkCollision(SDL_Rect A, SDL_Rect B);
    void render();
    void flipSprite(int* xMove, int* yMove, SDL_RendererFlip* flip, double* angle);
    void destroy();
    bool isRunning() const { return _running; }

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _snakeHead;
    SDL_Texture* _snakeBody;
    SDL_Texture* _appleTexture;
    SDL_Texture* _grass;
    SDL_Texture* _logo;
    SDL_Texture* _gameOver;

    std::vector<SnakeSegment> _snake;
    SnakeSegment _apple;
    int _score;
    bool _running;
    bool _moveX;
    bool _moveY;
    int _xMove;
    int _yMove;
    bool _canMove;
    bool _isValid;
    std::random_device _rd;
    std::mt19937 _gen;
    std::uniform_int_distribution<int> _dis;
    GameState _currentState;
};