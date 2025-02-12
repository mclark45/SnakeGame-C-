#include "SnakeGame.h"

int main(int argc, char* args[]) {
    SnakeGame game;
    
    if (!game.loadTextures()) {
        std::cerr << "Failed to load textures." << std::endl;
        return 1;
    }

    game.initSnake(100, 100, 4);

    while (game.isRunning()) {
        game._frameStart = SDL_GetTicks();

        game.handleEvents();
        game.update();
        game.render();

        // Set frame rate
        Uint32 frameTime = SDL_GetTicks() - game._frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    return 0;
}
