#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include "SDL_utils.h"

int main(int argc, char* argv[]) {
    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("SDL Game with Background",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer for the window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_image for PNG support
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load background image
    SDL_Surface* bgSurface = IMG_Load("background.png");
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "background.png", IMG_GetError());
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Create texture from surface
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    if (bgTexture == NULL) {
        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // Load character image
    SDL_Surface* charSurface = IMG_Load("character.png");
    if (charSurface == NULL) {
        printf("Unable to load character image! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(bgTexture);
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, charSurface);
    SDL_FreeSurface(charSurface);
    if (charTexture == NULL) {
        printf("Unable to create character texture! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(bgTexture);
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Define the destination rectangle for the character image
    SDL_Rect charRect;
    charRect.x = 350;   // x position (adjust as needed)
    charRect.y = 100;   // y position (adjust as needed)
    charRect.w = 100;   // width of the character (adjust as needed)
    charRect.h = 100;   // height of the character (adjust as needed)



    // Render the scene once
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);   // Render background
    SDL_RenderCopy(renderer, charTexture, NULL, &charRect);  // Render character
    SDL_RenderPresent(renderer);

    // Wait for a key press or quit event before exiting
    waitUntilKeyPressed();

    // Clean up resources
    SDL_DestroyTexture(charTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
