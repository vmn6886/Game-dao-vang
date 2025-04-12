#include <SDL.h>                              // Main SDL header (graphics, events, etc.)
#include <SDL_image.h>                        // SDL_image for image loading
#include <SDL_ttf.h>                          // SDL_ttf for fonts
#include <SDL_mixer.h>                        // SDL_mixer for audio
#include <stdio.h>                            // Standard I/O
#include <stdbool.h>                          // Boolean support
#include <math.h>                             // Math functions (sin, cos, etc.)
#include <stdlib.h>                           // Standard library (rand, srand, etc.)
#include <time.h>                             // Time functions (for seeding RNG)
#include "objects.h"                          // Include objects definitions
#include "high_scores.h"                      // Include high scores functions

#define PI 3.14159265358979323846             // Define PI constant

// Define hook states (if additional hook functions are not split yet, we define them here)
typedef enum { OSCILLATING, PULLING_DOWN, ROLLING_BACK, PULLING_GOLD, dynamite_MOVING, dynamite_EXPLOSION } HookState;

// Forward declarations for menu and UI functions.
int runMenu(SDL_Renderer* renderer, TTF_Font* font);
void showControls(SDL_Renderer* renderer, TTF_Font* font);
void showTargetScreen(SDL_Renderer* renderer, TTF_Font* font48, SDL_Texture* targetTexture, Mix_Music* targetMusic, int neededPoints);

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL)); // Seed random number generator
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { // Initialize SDL video subsystem
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) { // Initialize SDL_ttf
        printf("TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { // Initialize SDL_mixer
        printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return 1;
    }
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Load 24pt font
    if (!font) {
        printf("Failed to load font (24pt): %s\n", TTF_GetError());
        return 1;
    }
    TTF_Font* font48 = TTF_OpenFont("arial.ttf", 48); // Load 48pt font
    if (!font48) {
        printf("Failed to load font (48pt): %s\n", TTF_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Đào vàng", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1366, 768, SDL_WINDOW_SHOWN); // Create window
    if (!window) {
        printf("Window error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Create renderer
    if (!renderer) {
        printf("Renderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }
    int imgFlags = IMG_INIT_PNG; // For PNG images
    if (!(IMG_Init(imgFlags) & imgFlags)) { // Initialize SDL_image
        printf("SDL_image error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }
    // Load common assets.
    SDL_Surface* bgSurface = IMG_Load("background.png");
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    SDL_Surface* charSurface = IMG_Load("character.png");
    SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, charSurface);
    SDL_FreeSurface(charSurface);
    SDL_Surface* goldSurface = IMG_Load("gold.png");
    SDL_Texture* goldTexture = SDL_CreateTextureFromSurface(renderer, goldSurface);
    SDL_FreeSurface(goldSurface);
    SDL_Surface* hookSurface = IMG_Load("hook.png");
    SDL_Texture* hookTexture = SDL_CreateTextureFromSurface(renderer, hookSurface);
    SDL_FreeSurface(hookSurface);
    SDL_Surface* rockSurface = IMG_Load("rock.png");
    SDL_Texture* rockTexture = SDL_CreateTextureFromSurface(renderer, rockSurface);
    SDL_FreeSurface(rockSurface);
    SDL_Surface* dynamiteSurface = IMG_Load("dynamite.png");
    if (!dynamiteSurface)
        printf("Error loading dynamite.png: %s\n", IMG_GetError());
    SDL_Texture* dynamiteTexture = SDL_CreateTextureFromSurface(renderer, dynamiteSurface);
    SDL_FreeSurface(dynamiteSurface);
    SDL_Surface* implodeSurface = IMG_Load("explosion.png");
    if (!implodeSurface)
        printf("Error loading explosion.png: %s\n", IMG_GetError());
    SDL_Texture* implodeTexture = SDL_CreateTextureFromSurface(renderer, implodeSurface);
    SDL_FreeSurface(implodeSurface);
    SDL_Surface* mysbagSurface = IMG_Load("mysbag.png");
    if (!mysbagSurface)
        printf("Error loading mysbag.png: %s\n", IMG_GetError());
    SDL_Texture* mysbagTexture = SDL_CreateTextureFromSurface(renderer, mysbagSurface);
    SDL_FreeSurface(mysbagSurface);
    SDL_Surface* successSurface = IMG_Load("success.png");
    if (!successSurface)
        printf("Error loading success.png: %s\n", IMG_GetError());
    SDL_Texture* successTexture = SDL_CreateTextureFromSurface(renderer, successSurface);
    SDL_FreeSurface(successSurface);
    SDL_Surface* failureSurface = IMG_Load("failure.png");
    if (!failureSurface)
        printf("Error loading failure.png: %s\n", IMG_GetError());
    SDL_Texture* failureTexture = SDL_CreateTextureFromSurface(renderer, failureSurface);
    SDL_FreeSurface(failureSurface);
    SDL_Surface* targetSurface = IMG_Load("Target.png");
    if (!targetSurface)
        printf("Error loading Target.png: %s\n", IMG_GetError());
    SDL_Texture* targetTexture = SDL_CreateTextureFromSurface(renderer, targetSurface);
    SDL_FreeSurface(targetSurface);
    Mix_Music* targetMusic = Mix_LoadMUS("target.mp3");
    if (!targetMusic)
        printf("Error loading target.mp3: %s\n", Mix_GetError());
    // Main session loop.
    bool exitProgram = false;
    while (!exitProgram) {
        int menuResult = runMenu(renderer, font); // Display main menu
        if (menuResult == 1) { // If quit signal from menu
            exitProgram = true;
            break;
        }
        // Display target screen each time "Begin" is pressed
        showTargetScreen(renderer, font48, targetTexture, targetMusic, 400);
        // Initialize game session variables.
        SDL_Rect charRect = {583, 90, 200, 100};
        GoldObject golds[NUM_GOLDS];
        golds[0].type = GOLD_SMALL;  golds[0].rect = (SDL_Rect){50,300,20,20};   golds[0].active = true;
        golds[1].type = GOLD_SMALL;  golds[1].rect = (SDL_Rect){1250,320,20,20};  golds[1].active = true;
        golds[2].type = GOLD_SMALL;  golds[2].rect = (SDL_Rect){350,340,20,20};   golds[2].active = true;
        golds[3].type = GOLD_SMALL;  golds[3].rect = (SDL_Rect){600,360,20,20};   golds[3].active = true;
        golds[4].type = GOLD_SMALL;  golds[4].rect = (SDL_Rect){900,280,20,20};   golds[4].active = true;
        golds[5].type = GOLD_MEDIUM; golds[5].rect = (SDL_Rect){500,520,30,30};   golds[5].active = true;
        golds[6].type = GOLD_MEDIUM; golds[6].rect = (SDL_Rect){1150,640,30,30};  golds[6].active = true;
        golds[7].type = GOLD_MEDIUM; golds[7].rect = (SDL_Rect){800,700,30,30};   golds[7].active = true;
        golds[8].type = GOLD_BIG;    golds[8].rect = (SDL_Rect){450,600,60,60};   golds[8].active = true;
        golds[9].type = GOLD_BIG;    golds[9].rect = (SDL_Rect){1000,690,60,60};  golds[9].active = true;
        golds[10].type = GOLD_MYSTERY; golds[10].rect = (SDL_Rect){400,450,40,40};  golds[10].active = true;
        RockObject rocks[NUM_ROCKS];
        rocks[0].type = ROCK_BIG;   rocks[0].rect = (SDL_Rect){250,370,50,50};   rocks[0].active = true;
        rocks[1].type = ROCK_BIG;   rocks[1].rect = (SDL_Rect){550,380,50,50};   rocks[1].active = true;
        rocks[2].type = ROCK_SMALL; rocks[2].rect = (SDL_Rect){900,320,30,30};   rocks[2].active = true;
        rocks[3].type = ROCK_SMALL; rocks[3].rect = (SDL_Rect){1050,340,30,30};  rocks[3].active = true;
        float anchorX = charRect.x + charRect.w/2.0f;
        float anchorY = charRect.y + charRect.h/2.0f;
        float baseR = 70.0f;
        float maxAngle = 75.0f * (PI / 180.0f);
        float period = 2000.0f;
        float omega = 2*PI/(period/1000.0f);
        HookState hookState = OSCILLATING;
        Uint32 refTime = SDL_GetTicks();
        float currentAngle = 0.0f, storedAngle = 0.0f, currentR = baseR;
        float pullSpeed = 200.0f;
        float droppingSpeed = 1000.0f;
        float phaseOffset = 0.0f;
        float hookX, hookY;
        float scaleFactor = 0.05f;
        int hookW = (int)(928 * scaleFactor);
        int hookH = (int)(665 * scaleFactor);
        hookX = anchorX; hookY = anchorY + baseR;
        SDL_Point hookPivot = {(int)(465 * scaleFactor + 0.5f), (int)(77 * scaleFactor + 0.5f)};
        SDL_Rect hookCollision;
        hookCollision.w = 20; hookCollision.h = 20;
        int score = 0;
        bool isPullingRock = false;
        int pulledGoldIndex = -1, pulledRockIndex = -1;
        float dynamiteMoveTimeRemaining = 0.0f;
        float explosionTimeRemaining = 0.0f;
        float explosionX = 0.0f, explosionY = 0.0f;
        int availabledynamites = 0;
        float gameTimer = 60.0f;
        bool timeUp = false;
        bool quitSession = false;
        Uint32 lastTime = SDL_GetTicks();
        while (!quitSession) { // Game session loop
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    TTF_CloseFont(font);
                    TTF_CloseFont(font48);
                    Mix_CloseAudio();
                    IMG_Quit();
                    TTF_Quit();
                    SDL_Quit();
                    exit(0);
                }
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_DOWN && !timeUp) {
                        if (hookState == OSCILLATING) {
                            hookState = PULLING_DOWN;
                            storedAngle = currentAngle;
                        }
                    }
                    if (event.key.keysym.sym == SDLK_UP && !timeUp) {
                        if (hookState == PULLING_GOLD && availabledynamites > 0) {
                            if ((!isPullingRock && pulledGoldIndex != -1) ||
                                (isPullingRock && pulledRockIndex != -1)) {
                                hookState = dynamite_MOVING;
                                dynamiteMoveTimeRemaining = 0.05f;
                                explosionX = hookX;
                                explosionY = hookY;
                                if (!isPullingRock && pulledGoldIndex != -1)
                                    golds[pulledGoldIndex].active = false;
                                else if (isPullingRock && pulledRockIndex != -1)
                                    rocks[pulledRockIndex].active = false;
                                availabledynamites--;
                            }
                        }
                    }
                }
            }
            Uint32 currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;
            if (!timeUp) {
                gameTimer -= deltaTime;
                if (gameTimer <= 0) {
                    gameTimer = 0;
                    timeUp = true;
                    break;
                }
            }
            anchorX = charRect.x + charRect.w / 2.0f;
            anchorY = charRect.y + charRect.h / 2.0f;
            if (!timeUp) {
                switch (hookState) {
                    case OSCILLATING: {
                        float t = (currentTime - refTime) / 1000.0f;
                        currentAngle = maxAngle * sin(omega * t + phaseOffset);
                        currentR = baseR;
                        hookX = anchorX + currentR * sin(currentAngle);
                        hookY = anchorY + currentR * cos(currentAngle);
                        break;
                    }
                    case PULLING_DOWN: {
                        currentR += droppingSpeed * deltaTime;
                        hookX = anchorX + currentR * sin(storedAngle);
                        hookY = anchorY + currentR * cos(storedAngle);
                        if (hookY + hookH/2 >= 768 || hookX - hookW/2 <= 0 || hookX + hookW/2 >= 1366)
                            hookState = ROLLING_BACK;
                        break;
                    }
                    case ROLLING_BACK: {
                        currentR -= droppingSpeed * deltaTime;
                        hookX = anchorX + currentR * sin(storedAngle);
                        hookY = anchorY + currentR * cos(storedAngle);
                        if (currentR <= baseR + 1.0f) {
                            currentR = baseR;
                            phaseOffset = asin(storedAngle / maxAngle);
                            refTime = currentTime;
                            hookState = OSCILLATING;
                        }
                        break;
                    }
                    case PULLING_GOLD: {
                        float retractSpeed = isPullingRock ? pullSpeed * 0.5f : pullSpeed;
                        currentR -= retractSpeed * deltaTime;
                        hookX = anchorX + currentR * sin(storedAngle);
                        hookY = anchorY + currentR * cos(storedAngle);
                        SDL_Rect hookRect;
                        hookRect.x = (int)hookX - hookW/2;
                        hookRect.y = (int)hookY - hookH/2;
                        hookRect.w = hookW;
                        hookRect.h = hookH;
                        int hookCenterX = hookRect.x + hookPivot.x;
                        int hookCenterY = hookRect.y + hookPivot.y;
                        if (!isPullingRock && pulledGoldIndex != -1) {
                            golds[pulledGoldIndex].rect.x = hookCenterX - golds[pulledGoldIndex].rect.w/2;
                            golds[pulledGoldIndex].rect.y = hookCenterY - golds[pulledGoldIndex].rect.h/2;
                        } else if (isPullingRock && pulledRockIndex != -1) {
                            rocks[pulledRockIndex].rect.x = hookCenterX - rocks[pulledRockIndex].rect.w/2;
                            rocks[pulledRockIndex].rect.y = hookCenterY - rocks[pulledRockIndex].rect.h/2;
                        }
                        if (currentR <= baseR + 1.0f) {
                            currentR = baseR;
                            hookState = OSCILLATING;
                            phaseOffset = asin(storedAngle / maxAngle);
                            refTime = currentTime;
                            if (!isPullingRock && pulledGoldIndex != -1) {
                                if (golds[pulledGoldIndex].type == GOLD_MYSTERY) {
                                    int r = rand() % 100;
                                    if (r < 30)
                                        availabledynamites++;
                                    else if (r < 90)
                                        score += 100;
                                    else
                                        score += 250;
                                } else if (golds[pulledGoldIndex].type == GOLD_SMALL)
                                    score += 50;
                                else if (golds[pulledGoldIndex].type == GOLD_MEDIUM)
                                    score += 100;
                                else
                                    score += 200;
                                golds[pulledGoldIndex].active = false;
                                pulledGoldIndex = -1;
                            } else if (isPullingRock && pulledRockIndex != -1) {
                                score += (rocks[pulledRockIndex].type == ROCK_SMALL) ? 10 : 20;
                                rocks[pulledRockIndex].active = false;
                                pulledRockIndex = -1;
                            }
                            printf("Score: %d\n", score);
                        }
                        break;
                    }
                    case dynamite_MOVING: {
                        dynamiteMoveTimeRemaining -= deltaTime;
                        if (dynamiteMoveTimeRemaining <= 0) {
                            hookState = dynamite_EXPLOSION;
                            explosionTimeRemaining = 0.2f;
                        }
                        break;
                    }
                    case dynamite_EXPLOSION: {
                        explosionTimeRemaining -= deltaTime;
                        if (explosionTimeRemaining <= 0) {
                            currentR = baseR;
                            phaseOffset = asin(storedAngle / maxAngle);
                            refTime = currentTime;
                            hookState = OSCILLATING;
                            pulledGoldIndex = -1;
                            pulledRockIndex = -1;
                        }
                        break;
                    }
                }
            }
            SDL_Rect hookRect;
            hookRect.x = (int)hookX - hookW/2;
            hookRect.y = (int)hookY - hookH/2;
            hookRect.w = hookW;
            hookRect.h = hookH;
            hookCollision.x = (int)hookX - hookCollision.w/2;
            hookCollision.y = (int)hookY - hookCollision.h/2;
            int hookCenterX = hookRect.x + hookPivot.x;
            int hookCenterY = hookRect.y + hookPivot.y;
            if (!timeUp && hookState == PULLING_DOWN) {
                float bestDist = 1e9f;
                bool found = false;
                int bestIndex = -1;
                bool bestIsRock = false;
                for (int i = 0; i < NUM_GOLDS; i++) {
                    if (golds[i].active && SDL_HasIntersection(&hookCollision, &golds[i].rect)) {
                        int objCenterX = golds[i].rect.x + golds[i].rect.w/2;
                        int objCenterY = golds[i].rect.y + golds[i].rect.h/2;
                        float dx = (float)(objCenterX - hookCenterX);
                        float dy = (float)(objCenterY - hookCenterY);
                        float dist = dx * dx + dy * dy;
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestIndex = i;
                            bestIsRock = false;
                            found = true;
                        }
                    }
                }
                for (int i = 0; i < NUM_ROCKS; i++) {
                    if (rocks[i].active && SDL_HasIntersection(&hookCollision, &rocks[i].rect)) {
                        int objCenterX = rocks[i].rect.x + rocks[i].rect.w/2;
                        int objCenterY = rocks[i].rect.y + rocks[i].rect.h/2;
                        float dx = (float)(objCenterX - hookCenterX);
                        float dy = (float)(objCenterY - hookCenterY);
                        float dist = dx * dx + dy * dy;
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestIndex = i;
                            bestIsRock = true;
                            found = true;
                        }
                    }
                }
                if (found) {
                    hookState = PULLING_GOLD;
                    if (bestIsRock) {
                        isPullingRock = true;
                        pulledRockIndex = bestIndex;
                    } else {
                        isPullingRock = false;
                        pulledGoldIndex = bestIndex;
                    }
                }
            }
            SDL_RenderClear(renderer);
            if (!timeUp) {
                SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
                for (int i = 0; i < NUM_GOLDS; i++) {
                    if (golds[i].active) {
                        if (golds[i].type == GOLD_MYSTERY)
                            SDL_RenderCopy(renderer, mysbagTexture, NULL, &golds[i].rect);
                        else
                            SDL_RenderCopy(renderer, goldTexture, NULL, &golds[i].rect);
                    }
                }
                for (int i = 0; i < NUM_ROCKS; i++) {
                    if (rocks[i].active)
                        SDL_RenderCopy(renderer, rockTexture, NULL, &rocks[i].rect);
                }
                SDL_RenderCopy(renderer, charTexture, NULL, &charRect);
                float angleDeg = -(currentAngle * 180.0f / PI);
                if (hookState == dynamite_MOVING) {
                    SDL_Rect dynamiteRect = { (int)hookX - hookW/2, (int)hookY - hookH/2, hookW, hookH };
                    SDL_RenderCopyEx(renderer, dynamiteTexture, NULL, &dynamiteRect, angleDeg, &hookPivot, SDL_FLIP_NONE);
                } else if (hookState == dynamite_EXPLOSION) {
                    SDL_Rect explosionRect;
                    explosionRect.x = (int)explosionX - hookW/2;
                    explosionRect.y = (int)explosionY - hookH/2;
                    explosionRect.w = 100;
                    explosionRect.h = 100;
                    SDL_RenderCopy(renderer, implodeTexture, NULL, &explosionRect);
                } else {
                    SDL_RenderCopyEx(renderer, hookTexture, NULL, &hookRect, angleDeg, &hookPivot, SDL_FLIP_NONE);
                }
                int hookPivotScreenX = hookRect.x + hookPivot.x;
                int hookPivotScreenY = hookRect.y + hookPivot.y;
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, (int)anchorX, (int)anchorY, hookPivotScreenX, hookPivotScreenY);
                char scoreText[32];
                sprintf(scoreText, "Score: %d", score);
                SDL_Color whiteColor = {255, 255, 255, 255};
                SDL_Surface* textSurface = TTF_RenderText_Blended(font, scoreText, whiteColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    SDL_FreeSurface(textSurface);
                    SDL_Rect textRect = {10, 10, 0, 0};
                    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                for (int i = 0; i < availabledynamites; i++) {
                    SDL_Rect dRect = { charRect.x + charRect.w + i * 50, 50, 50, 50 };
                    SDL_RenderCopy(renderer, dynamiteTexture, NULL, &dRect);
                }
                char timerText[32];
                int minutes = ((int)gameTimer) / 60;
                int seconds = ((int)gameTimer) % 60;
                sprintf(timerText, "Time: %d:%02d", minutes, seconds);
                SDL_Surface* timerSurface = TTF_RenderText_Blended(font, timerText, whiteColor);
                if (timerSurface) {
                    SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
                    SDL_FreeSurface(timerSurface);
                    SDL_Rect timerRect = {10, 40, 0, 0};
                    SDL_QueryTexture(timerTexture, NULL, NULL, &timerRect.w, &timerRect.h);
                    SDL_RenderCopy(renderer, timerTexture, NULL, &timerRect);
                    SDL_DestroyTexture(timerTexture);
                }
            } else {
                SDL_Rect fullScreen = {0, 0, 1366, 768};
                SDL_RenderCopy(renderer, failureTexture, NULL, &fullScreen);
            }
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        } // End of game session loop
        SDL_Rect fullScreenRect = {0, 0, 1366, 768};
        if (score >= 400) {
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, successTexture, NULL, &fullScreenRect);
            SDL_RenderPresent(renderer);
            SDL_Delay(4000);
            // Win: After showing success, return to main menu.
        } else {
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, failureTexture, NULL, &fullScreenRect);
            SDL_RenderPresent(renderer);
            SDL_Delay(4000);
        }
        updateHighScores(score);

    } // End of main session loop (returns to menu after each game session)
    SDL_DestroyTexture(hookTexture);
    SDL_DestroyTexture(rockTexture);
    SDL_DestroyTexture(goldTexture);
    SDL_DestroyTexture(charTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(dynamiteTexture);
    SDL_DestroyTexture(implodeTexture);
    SDL_DestroyTexture(mysbagTexture);
    SDL_DestroyTexture(successTexture);
    SDL_DestroyTexture(failureTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_CloseFont(font48);
    Mix_FreeMusic(targetMusic);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}

void showTargetScreen(SDL_Renderer* renderer, TTF_Font* font48, SDL_Texture* targetTexture, Mix_Music* targetMusic, int neededPoints) {
    if (targetMusic)
        Mix_PlayMusic(targetMusic, 1);
    char targetText[64];
    sprintf(targetText, "%d points", neededPoints);
    SDL_Color white = {255,255,255,255};
    SDL_Surface* textSurface = TTF_RenderText_Blended(font48, targetText, white);
    if (!textSurface) {
        printf("Error rendering target text: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    int textW, textH;
    SDL_QueryTexture(textTexture, NULL, NULL, &textW, &textH);
    Uint32 startTime = SDL_GetTicks();
    while (SDL_GetTicks() - startTime < 4000) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, targetTexture, NULL, NULL);
        SDL_Rect textRect = { (1366 - textW)/2, (768 - textH)/2, textW, textH };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyTexture(textTexture);
}

void showControls(SDL_Renderer* renderer, TTF_Font* font) {
    bool done = false;
    SDL_Event e;
    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                done = true;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                done = true;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderClear(renderer);
        SDL_Color white = {255,255,255,255};
        const char* line1 = "Controls:";
        const char* line2 = "Up button = Dynamite";
        const char* line3 = "Down button = Lower Mining Crank";
        const char* line4 = "(Press ESC to go back)";
        auto renderLine = [renderer, font, white](const char* text, int yOffset) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, text, white);
            if (!surf) return;
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            SDL_Rect rect = {0,0,0,0};
            SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);
            rect.x = (1366 - rect.w)/2;
            rect.y = yOffset;
            SDL_RenderCopy(renderer, tex, NULL, &rect);
            SDL_DestroyTexture(tex);
        };
        renderLine(line1, 150);
        renderLine(line2, 200);
        renderLine(line3, 240);
        renderLine(line4, 300);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

int runMenu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Surface* menuBGSurface = IMG_Load("daovang.png");
    if (!menuBGSurface) {
        printf("Error loading daovang.png: %s\n", IMG_GetError());
        return 1;
    }
    SDL_Texture* menuBGTexture = SDL_CreateTextureFromSurface(renderer, menuBGSurface);
    SDL_FreeSurface(menuBGSurface);
    SDL_Rect goldRect = {420,150,200,200};
    SDL_Rect controlRect = {300,500,200,50};
    SDL_Rect scoresRect = {500,500,200,50};
    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyTexture(menuBGTexture);
                return 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                if (mx >= goldRect.x && mx <= goldRect.x + goldRect.w && my >= goldRect.y && my <= goldRect.y + goldRect.h) {
                    SDL_DestroyTexture(menuBGTexture);
                    return 0;
                }
                if (mx >= controlRect.x && mx <= controlRect.x + controlRect.w && my >= controlRect.y && my <= controlRect.y + controlRect.h)
                    showControls(renderer, font);
                if (mx >= scoresRect.x && mx <= scoresRect.x + scoresRect.w && my >= scoresRect.y && my <= scoresRect.y + scoresRect.h)
                    showHighScores(renderer, font);
            }
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuBGTexture, NULL, NULL);
        SDL_SetRenderDrawColor(renderer, 0,255,0,100);
        SDL_RenderFillRect(renderer, &controlRect);
        SDL_SetRenderDrawColor(renderer, 0,0,255,100);
        SDL_RenderFillRect(renderer, &scoresRect);
        SDL_Color white = {255,255,255,255};
        SDL_Surface* bgnSurf = TTF_RenderText_Blended(font, "Begin", white);
        if (bgnSurf) {
            SDL_Texture* bgnText = SDL_CreateTextureFromSurface(renderer, bgnSurf);
            SDL_FreeSurface(bgnSurf);
            SDL_Rect bgnTextRect = {goldRect.x, goldRect.y, 0, 0};
            SDL_QueryTexture(bgnText, NULL, NULL, &bgnTextRect.w, &bgnTextRect.h);
            bgnTextRect.x += (goldRect.w - bgnTextRect.w) / 2;
            bgnTextRect.y += (goldRect.h - bgnTextRect.h) / 2;
            SDL_RenderCopy(renderer, bgnText, NULL, &bgnTextRect);
            SDL_DestroyTexture(bgnText);
        }
        SDL_Surface* ctrlSurf = TTF_RenderText_Blended(font, "Control", white);
        if (ctrlSurf) {
            SDL_Texture* ctrlText = SDL_CreateTextureFromSurface(renderer, ctrlSurf);
            SDL_FreeSurface(ctrlSurf);
            SDL_Rect ctrlTextRect = {controlRect.x, controlRect.y, 0, 0};
            SDL_QueryTexture(ctrlText, NULL, NULL, &ctrlTextRect.w, &ctrlTextRect.h);
            ctrlTextRect.x += (controlRect.w - ctrlTextRect.w) / 2;
            ctrlTextRect.y += (controlRect.h - ctrlTextRect.h) / 2;
            SDL_RenderCopy(renderer, ctrlText, NULL, &ctrlTextRect);
            SDL_DestroyTexture(ctrlText);
        }
        SDL_Surface* scoresSurf = TTF_RenderText_Blended(font, "High Scores", white);
        if (scoresSurf) {
            SDL_Texture* scoresText = SDL_CreateTextureFromSurface(renderer, scoresSurf);
            SDL_FreeSurface(scoresSurf);
            SDL_Rect scoresTextRect = {scoresRect.x, scoresRect.y, 0, 0};
            SDL_QueryTexture(scoresText, NULL, NULL, &scoresTextRect.w, &scoresTextRect.h);
            scoresTextRect.x += (scoresRect.w - scoresTextRect.w) / 2;
            scoresTextRect.y += (scoresRect.h - scoresTextRect.h) / 2;
            SDL_RenderCopy(renderer, scoresText, NULL, &scoresTextRect);
            SDL_DestroyTexture(scoresText);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyTexture(menuBGTexture);
    return 1;
}
