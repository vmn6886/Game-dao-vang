#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "SDL_utils.h"
using namespace std;

const int SCREEN_WIDTH = 1366;
const int SCREEN_HEIGHT = 768;
const string WINDOW_TITLE = "Dao vang";

int main(int argc, char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    initSDL(window, renderer, WINDOW_TITLE.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderClear(renderer);
    SDL_Rect filledsquare;
    filledsquare.x = 266;
    filledsquare.y = 200;
    filledsquare.w = 200;
    filledsquare.h = 200;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &filledsquare);
    SDL_RenderPresent(renderer);
    waitUntilKeyPressed();
    quitSDL(window, renderer);
    return 0;
}