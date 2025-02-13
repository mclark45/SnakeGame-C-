#include "SnakeGame.h"

SnakeGame::SnakeGame() :
    _running(true),
    _moveX(true),
    _moveY(false),
    _xMove(speed),
    _yMove(0),
    _canMove(true),
    _isValid(true),
    _currentState(GameState::MENU)
{
    init();
}

SnakeGame::~SnakeGame() {
    destroy();
}

void SnakeGame::init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		_running = false;
	}

	_window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (_window == NULL) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		_running = false;
	}

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (_renderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		_running = false;
	}

    srand(time(NULL));
    for (int i = 0; i < SCREEN_HEIGHT / 32; i++) {
        for (int j = 0; j < SCREEN_WIDTH / 32; j++) {
            rotations[i][j] = (rand() % 4) * 90;
        }
    }
}

void SnakeGame::initSnake(int x, int y, int length) {
    if (length <= 0) {
        // Handle invalid length
        std::cerr << "Invalid snake length: " << length << std::endl;
        return;
    }

    for (int i = 0; i < length; i++) {
        SnakeSegment snake = { x - i * 32, y };
        _snake.push_back({
            SCREEN_WIDTH / 2 - (SCREEN_WIDTH / 2) % pixel_size,
            SCREEN_HEIGHT / 2 - (SCREEN_HEIGHT / 2) % pixel_size }
        );
    }
}

void SnakeGame::setApplePosition(int x, int y) {
    std::tie(_apple.x, _apple.y) = applePosition(_apple);
}

bool SnakeGame::loadTextures() {
    _snakeHead = IMG_LoadTexture(_renderer, "snakeHead.png");
    if (_snakeHead == NULL) {
        printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
        _running = false;
        return false;
    }

    _snakeBody = IMG_LoadTexture(_renderer, "snakeBody.png");
    if (_snakeBody == NULL) {
        printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
        _running = false;
        return false;
    }

    _appleTexture = IMG_LoadTexture(_renderer, "apple.png");
    if (_appleTexture == NULL) {
        printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
        _running = false;
        return false;
    }

    _grass = IMG_LoadTexture(_renderer, "grass.png");
    if (_grass == NULL) {
        printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
        _running = false;
        return false;
    }

    _logo = IMG_LoadTexture(_renderer, "snakeLogo.png");
    if (_logo == NULL) {
        printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
        _running = false;
        return false;
    }

    _gameOver = IMG_LoadTexture(_renderer, "SnakeGameOver.png");
    if (_gameOver == NULL) {
        printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
        _running = false;
        return false;
    }

    return true;
}

void SnakeGame::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            _running = false;
        }

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_d:
                if (_moveY && _canMove) {
                    _xMove = speed;
                    _yMove = 0;
                    _moveX = true;
                    _moveY = false;
                    _canMove = false;
                }
                break;

            case SDLK_a:
                if (_moveY && _canMove) {
                    _xMove = -speed;
                    _yMove = 0;
                    _moveX = true;
                    _moveY = false;
                    _canMove = false;
                }
                break;

            case SDLK_s:
                if (_moveX && _canMove) {
                    _xMove = 0;
                    _yMove = speed;
                    _moveX = false;
                    _moveY = true;
                    _canMove = false;
                }
                break;

            case SDLK_w:
                if (_moveX && _canMove) {
                    _xMove = 0;
                    _yMove = -speed;
                    _moveX = false;
                    _moveY = true;
                    _canMove = false;
                }
                break;

            case SDLK_SPACE:
                if (_currentState == GameState::MENU) {
                    _xMove = speed;
                    _yMove = 0;
                    _moveX = true;
                    _moveY = false;
                    _canMove = false;
                    _score = 0;
                    _snake.clear();
                    initSnake(100, 100, 4);
                    std::tie(_apple.x, _apple.y) = applePosition(_apple);
                    _currentState = GameState::GAME;
                }
                else if (_currentState == GameState::GAME_OVER) {
                    _currentState = GameState::MENU;
                }
                break;

            default:
                break;
            }
        }
    }
}

void SnakeGame::update() {
    if (_currentState == GameState::GAME) {
        if (!_snake.empty()) {
            // Store the previous positions of the snake's segments
            std::vector<SnakeSegment> prevPositions = _snake;

            // Move the head of the snake
            _snake[0].x += _xMove * pixel_size;
            _snake[0].y += _yMove * pixel_size;

            // Align snake with grid
            _snake[0].x = (_snake[0].x / pixel_size) * pixel_size;
            _snake[0].y = (_snake[0].y / pixel_size) * pixel_size;

            // Move the body of the snake
            for (int i = 1; i < _snake.size(); i++) {
                _snake[i].x = prevPositions[i - 1].x;
                _snake[i].y = prevPositions[i - 1].y;
            }


            _canMove = true;

            // Check for collision with wall
            if (_snake[0].x < 0 || _snake[0].x >= SCREEN_WIDTH || _snake[0].y < 0 || _snake[0].y >= SCREEN_HEIGHT) {
                _currentState = GameState::GAME_OVER;
            }

            // Check for collision with apple
            SDL_Rect snakeHeadRect = { _snake[0].x, _snake[0].y, pixel_size, pixel_size };
            SDL_Rect appleRect = { _apple.x, _apple.y, pixel_size, pixel_size };
            if (checkCollision(snakeHeadRect, appleRect)) {
                _score += 10;

                // Generate new apple position
                std::tie(_apple.x, _apple.y) = applePosition(_apple);

                // Add new segment to snake
                if (!_snake.empty()) {
                    _snake.push_back({ _snake.back().x, _snake.back().y });
                }
            }

            // Check for collision with self
            for (int i = 1; i < _snake.size(); i++) {
                SDL_Rect snakeBodyRect = { _snake[i].x, _snake[i].y, pixel_size, pixel_size };
                if (checkCollision(snakeHeadRect, snakeBodyRect)) {
                    _currentState = GameState::GAME_OVER;
                }
            }
        }
    }
}

bool SnakeGame::checkCollision(SDL_Rect A, SDL_Rect B)
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = A.x;
    rightA = A.x + A.w;
    topA = A.y;
    bottomA = A.y + A.h;

    //Calculate the sides of rect B
    leftB = B.x;
    rightB = B.x + B.w;
    topB = B.y;
    bottomB = B.y + B.h;

    //If any of the sides from A are outside of B
    if (bottomA <= topB)
    {
        return false;
    }

    if (topA >= bottomB)
    {
        return false;
    }

    if (rightA <= leftB)
    {
        return false;
    }

    if (leftA >= rightB)
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
}

std::tuple<int, int> SnakeGame::applePosition(SnakeSegment apple) {
    _dis = std::uniform_int_distribution<int>(0, (SCREEN_WIDTH / pixel_size) - 1);
    apple.x = _dis(_gen) * pixel_size;
    _dis = std::uniform_int_distribution<int>(0, (SCREEN_HEIGHT / pixel_size) - 1);
    apple.y = _dis(_gen) * pixel_size;

    SDL_Rect appleRect = { apple.x, apple.y, pixel_size, pixel_size };

    bool isValidPosition = false;
    while (!isValidPosition) {
        isValidPosition = true;
        for (int i = 0; i < _snake.size(); i++) {
            SDL_Rect snakeBodyRect = { _snake[i].x, _snake[i].y, pixel_size, pixel_size };

            if (checkCollision(appleRect, snakeBodyRect)) {
                isValidPosition = false;
                _dis = std::uniform_int_distribution<int>(0, (SCREEN_WIDTH / pixel_size) - 1);
                apple.x = _dis(_gen) * pixel_size;
                _dis = std::uniform_int_distribution<int>(0, (SCREEN_HEIGHT / pixel_size) - 1);
                apple.y = _dis(_gen) * pixel_size;
                appleRect = { apple.x, apple.y, pixel_size, pixel_size };
                break;
            }
        }
    }

    return std::make_tuple(apple.x, apple.y);
}

void SnakeGame::render() {
    SDL_SetRenderDrawColor(_renderer, 1, 50, 32, 1);
    SDL_RenderClear(_renderer);

    // Render background
    for (int i = 0; i < SCREEN_HEIGHT / 32; i++) {
        for (int j = 0; j < SCREEN_WIDTH / 32; j++) {
            SDL_Rect rect = { j * 32, i * 32, 32, 32 };
            double angle = rotations[i][j];
            SDL_RenderCopyEx(_renderer, _grass, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        }
    }

    if (_currentState == GameState::MENU) {
        SDL_Rect logoRect = { (SCREEN_WIDTH - 300) / 2, (SCREEN_HEIGHT - 100) / 2, 300, 100 };
        SDL_RenderCopy(_renderer, _logo, NULL, &logoRect);
        SDL_RenderPresent(_renderer);
    }
    else if (_currentState == GameState::GAME) {

        // Render apple
        SDL_Rect appleRect = { _apple.x, _apple.y, pixel_size, pixel_size };
        SDL_RenderCopy(_renderer, _appleTexture, NULL, &appleRect);

        // Render snake
        if (!_snake.empty()) {
            for (int i = 0; i < _snake.size(); i++) {
                SDL_Rect rect = { _snake[i].x, _snake[i].y, 32, 32 };
                SDL_RendererFlip flip = SDL_FLIP_NONE;
                double angle = 0.0;
                if (i == 0) {
                    flipSprite(&_xMove, &_yMove, &flip, &angle);
                    SDL_RenderCopyEx(_renderer, _snakeHead, NULL, &rect, angle, NULL, flip);
                }
                else {
                    SDL_RenderCopyEx(_renderer, _snakeBody, NULL, &rect, angle, NULL, flip);
                }
            }
        }
        SDL_RenderPresent(_renderer);
    }
    else if (_currentState == GameState::GAME_OVER) {
        SDL_Rect gameOverRect = { (SCREEN_WIDTH - 300) / 2, (SCREEN_HEIGHT - 300) / 2, 500, 300 };
        SDL_RenderCopy(_renderer, _gameOver, NULL, &gameOverRect);
        SDL_RenderPresent(_renderer);
    }
}

void SnakeGame::flipSprite(int* xMove, int* yMove, SDL_RendererFlip* flip, double* angle) {
    if (*xMove < 0 && *yMove == 0)
        *flip = SDL_FLIP_HORIZONTAL;
    else if (*xMove > 0 && *yMove == 0)
        *flip = SDL_FLIP_NONE;
    else if (*yMove > 0 && *xMove == 0)
        *angle = 90.0;
    else if (*yMove < 0 && *xMove == 0)
        *angle = -90.0;
}

GameState SnakeGame::getCurrentState() {
    return _currentState;
}

void SnakeGame::destroy() {
    SDL_DestroyTexture(_logo);
    SDL_DestroyTexture(_grass);
    SDL_DestroyTexture(_appleTexture);
    SDL_DestroyTexture(_snakeBody);
    SDL_DestroyTexture(_snakeHead);
    SDL_DestroyTexture(_gameOver);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}