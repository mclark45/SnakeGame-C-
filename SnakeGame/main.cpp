#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <iostream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FPS = 10;
const int frameDelay = 1000 / FPS;
const int speed = 1;
int score = 0;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* snakeHead = NULL;
SDL_Texture* snakeBody = NULL;
SDL_Texture* apple = NULL;
SDL_Texture* grass = NULL;
SDL_Texture* logo = NULL;

bool init();
bool loadTextures();
bool screenBarrier(SDL_Rect* snake);
void flipSprite(float* xMove, float* yMove, SDL_RendererFlip* flip, double* angle);
bool checkCollision(SDL_Rect a, SDL_Rect b);
void render(float* xMove, float* yMove, std::vector<SDL_Rect> background, std::vector<SDL_Rect> snake, SDL_Rect* applePos);
void renderLogo(SDL_Renderer* renderer, SDL_Texture* texture);
void destroy();

int main(int argc, char* args[]) {

	// start sdl and create window
	if (!init()) {
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return -1;
	}

	// load in all textures
	if (!loadTextures()) {
		printf("SDL could not load textures! SDL Error: %s\n", SDL_GetError());
		return -1;
	}

	bool running = true;
	SDL_Event event;
	float posX = SCREEN_WIDTH / 2;
	float posY = SCREEN_HEIGHT / 2;
	int pixelSize = 32;
	float xMove = speed;
	float yMove = 0;
	bool moveX = true;
	bool moveY = false;
	Uint32 frameStart;
	Uint32 delay = 0;
	int frameTime;
	bool canMove = true;
	bool isValid = true;
	std::vector<SDL_Rect> snake(4);
	std::vector<SDL_Rect> background(80);
	Uint32 startTime = SDL_GetTicks();
	bool showLogo = true;

	// fill background vector
	for (int i = 0; i < background.size(); i++) {
		int row = i / 10;
		int col = i % 10;

		background[i].x = col * pixelSize * 2;
		background[i].y = row * pixelSize * 2;
		background[i].w = pixelSize * 2;
		background[i].h = pixelSize * 2;
	}

	// initial snake body
	for (int i = 0; i < snake.size(); i++) {
		snake[i].x = posX - i * pixelSize;
		snake[i].y = posY;
		snake[i].w = pixelSize;
		snake[i].h = pixelSize;
	}

	// apple rect
	SDL_Rect applePos = {
		applePos.x = rand() % SCREEN_WIDTH - pixelSize,
		applePos.y = rand() % SCREEN_HEIGHT - pixelSize,
		applePos.w = pixelSize,
		applePos.h = pixelSize
	};
	
	// game loop
	while (running) {

		// Show logo for 5 seconds
		while (showLogo) {
			Uint32 currentTime = SDL_GetTicks();
			if (currentTime - startTime > 5000) {
				showLogo = false;
			}

			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(renderer);

			renderLogo(renderer, logo);

			SDL_RenderPresent(renderer);
		}

		frameStart = SDL_GetTicks();

		// get user input
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}

			if (event.type == SDL_KEYDOWN) {

				switch (event.key.keysym.sym)
				{
				case SDLK_d:
					if (moveY && canMove) {
						xMove = speed;
						yMove = 0;
						moveX = true;
						moveY = false;
						canMove = false;
					}
					break;

				case SDLK_a:
					if (moveY && canMove) {
						xMove = -speed;
						yMove = 0;
						moveX = true;
						moveY = false;
						canMove = false;
					}
					break;

				case SDLK_s:
					if (moveX && canMove) {
						xMove = 0;
						yMove = speed;
						moveX = false;
						moveY = true;
						canMove = false;
					}
					break;

				case SDLK_w:
					if (moveX && canMove) {
						xMove = 0;
						yMove = -speed;
						moveX = false;
						moveY = true;
						canMove = false;
					}
					break;

				default:
					break;
				}
			}
		}

		SDL_Rect temp;
		SDL_Rect prev = snake[0];

		// move head of snake
		snake[0].x += xMove * pixelSize;
		snake[0].y += yMove * pixelSize;

		// move the body
		for (int i = 1; i < snake.size(); i++) {
			temp = snake[i];
			snake[i] = prev;
			prev = temp;
		}

		canMove = true;

		if (!screenBarrier(&snake[0])) {
			running = false;
			std::cout << "Game Over!" << std::endl;
			break;
		}

		// apple collision detection
		if (checkCollision(snake[0], applePos)) {

			do {
				isValid = true;
				applePos.x = rand() % (SCREEN_WIDTH - pixelSize);
				applePos.y = rand() % (SCREEN_HEIGHT - pixelSize);

				SDL_Rect newSegement = {
				newSegement.x = posX - snake[snake.size() - 1].x * pixelSize,
				newSegement.y = posY - snake[snake.size() - 1].y * pixelSize,
				newSegement.w = pixelSize,
				newSegement.h = pixelSize
				};
				score += 10;

				snake.push_back(newSegement);

				for (SDL_Rect& segment : snake) {
					if (segment.x == applePos.x && segment.y == applePos.y) {
						isValid - false;
						break;
					}
				}
			} while (!isValid);
		}

		// snake collision dectection
		for (int i = 1; i < snake.size(); i++) {
			if (checkCollision(snake[0], snake[i])) {
				running = false;
				std::cout << "Game Over!" << std::endl;
				break;
			}
		}

		// display graphics
		render(&xMove, &yMove, background, snake, &applePos);

		// set frame rate
		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime) {
			SDL_Delay(frameDelay - frameTime);
		}

		std::cout << "Score: " << score << std::endl;
	}

	// free resources and close sdl
	destroy();
	return 0;
}

bool init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

bool loadTextures() {
	snakeHead = IMG_LoadTexture(renderer, "snakeHead.png");
	if (snakeHead == NULL) {
		printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	snakeBody = IMG_LoadTexture(renderer, "snakeBody.png");
	if (snakeBody == NULL) {
		printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	apple = IMG_LoadTexture(renderer, "apple.png");
	if (apple == NULL) {
		printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	grass = IMG_LoadTexture(renderer, "grass.png");
	if (grass == NULL) {
		printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	logo = IMG_LoadTexture(renderer, "snakeLogo.png");
	if (grass == NULL) {
		printf("SDL could not load texture! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

bool screenBarrier(SDL_Rect* snake) {
	if (snake->x < 0) {
		snake->x = SCREEN_WIDTH - snake->w;
		return false;
	}
	else if (snake->x + snake->w > SCREEN_WIDTH) {
		return false;
	}
	if (snake->y < 0) {
		return false;
	}
	else if (snake->y + snake->h > SCREEN_HEIGHT) {
		return false;
	}
}

void flipSprite(float* xMove, float* yMove, SDL_RendererFlip* flip, double* angle){
	if (*xMove < 0 && *yMove == 0)
		*flip = SDL_FLIP_HORIZONTAL;
	else if (*xMove > 0 && *yMove == 0)
		*flip = SDL_FLIP_NONE;
	else if (*yMove > 0 && *xMove == 0)
		*angle = 90.0;
	else if (*yMove < 0 && *xMove == 0)
		*angle = -90.0;
}

bool checkCollision(SDL_Rect A, SDL_Rect B)
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

void render(float* xMove, float* yMove, std::vector<SDL_Rect> background, std::vector<SDL_Rect> snake, SDL_Rect* applePos) {
	SDL_SetRenderDrawColor(renderer, 1, 50, 32, 1);
	SDL_RenderClear(renderer);

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	double angle = 0.0;

	flipSprite(xMove, yMove, &flip, &angle);

	for (int i = 0; i < background.size(); i++) {
		SDL_RenderCopy(renderer, grass, NULL, &background[i]);
	}

	SDL_RenderCopy(renderer, apple, NULL, applePos);

	for (int i = 0; i < snake.size(); i++) {
		if (i == 0)
			SDL_RenderCopyEx(renderer, snakeHead, NULL, &snake[i], angle, NULL, flip);
		else
			SDL_RenderCopyEx(renderer, snakeBody, NULL, &snake[i], angle, NULL, flip);
	}

	SDL_RenderPresent(renderer);
}

void destroy() {
	SDL_DestroyTexture(logo);
	SDL_DestroyTexture(grass);
	SDL_DestroyTexture(apple);
	SDL_DestroyTexture(snakeBody);
	SDL_DestroyTexture(snakeHead);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	grass = NULL;
	apple = NULL;
	snakeBody = NULL;
	snakeHead = NULL;
	renderer = NULL;
	window = NULL;
	SDL_Quit();
}

void renderLogo(SDL_Renderer* renderer, SDL_Texture* texture) {
	SDL_Rect renderQuad = { ((SCREEN_WIDTH - 256) / 2) - 50, ((SCREEN_HEIGHT - 256) / 2) + 50, 400, 128 };
	SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
}
