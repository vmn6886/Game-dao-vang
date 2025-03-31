#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// Define PI.
#define PI 3.14159265358979323846

// Define hook states.
typedef enum {
    OSCILLATING,
    PULLING_DOWN,
    ROLLING_BACK,
    PULLING_GOLD,
    DINAMO_MOVING,    // new: dinamo is moving toward the pulled object
    DINAMO_EXPLOSION  // new: explosion effect (explosion.png) shown
} HookState;

// Define gold and rock types.
typedef enum { GOLD_SMALL, GOLD_MEDIUM, GOLD_BIG } GoldType;
typedef enum { ROCK_SMALL, ROCK_BIG } RockType;

// Structures for game objects.
typedef struct {
    SDL_Rect rect;
    GoldType type;
    bool active;
} GoldObject;

typedef struct {
    SDL_Rect rect;
    RockType type;
    bool active;
} RockObject;

// Constants for number of objects.
#define NUM_GOLDS 3
#define NUM_ROCKS 3

// Forward declarations.
int runMenu(SDL_Renderer* renderer, TTF_Font* font);
void showControls(SDL_Renderer* renderer, TTF_Font* font);
void showHighScores(SDL_Renderer* renderer, TTF_Font* font);

int main(int argc, char* argv[])
{
    // ------------------- Initialization -------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL Game with Hook",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // ------------------- Run the Main Menu -------------------
    int menuResult = runMenu(renderer, font);
    if (menuResult == 1) {
        TTF_CloseFont(font);
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    // If we get here, user clicked the gold in the menu to start the game.

    // ------------------- Load Game Assets -------------------
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

    // Load new textures for dinamo and explosion effect.
    SDL_Surface* dinamoSurface = IMG_Load("dynamite.png");
    if(!dinamoSurface) {
        printf("Error loading dynamite.png: %s\n", IMG_GetError());
    }
    SDL_Texture* dinamoTexture = SDL_CreateTextureFromSurface(renderer, dinamoSurface);
    SDL_FreeSurface(dinamoSurface);

    SDL_Surface* implodeSurface = IMG_Load("explosion.png");
    if(!implodeSurface) {
        printf("Error loading explosion.png: %s\n", IMG_GetError());
    }
    SDL_Texture* implodeTexture = SDL_CreateTextureFromSurface(renderer, implodeSurface);
    SDL_FreeSurface(implodeSurface);

    // ------------------- Initialize Objects -------------------
    SDL_Rect charRect = {300, 15, 200, 100}; // Character

    GoldObject golds[NUM_GOLDS];
    golds[0].type = GOLD_SMALL;  golds[0].rect = (SDL_Rect){400, 300, 30, 30};  golds[0].active = true;
    golds[1].type = GOLD_MEDIUM; golds[1].rect = (SDL_Rect){250, 450, 50, 50};  golds[1].active = true;
    golds[2].type = GOLD_BIG;    golds[2].rect = (SDL_Rect){300, 500, 80, 80};  golds[2].active = true;

    RockObject rocks[NUM_ROCKS];
    rocks[0].type = ROCK_SMALL;  rocks[0].rect = (SDL_Rect){500, 400, 30, 30};  rocks[0].active = true;
    rocks[1].type = ROCK_BIG;    rocks[1].rect = (SDL_Rect){550, 450, 50, 50};  rocks[1].active = true;
    rocks[2].type = ROCK_BIG;    rocks[2].rect = (SDL_Rect){600, 500, 50, 50};  rocks[2].active = true;

    // ------------------- Hook Parameters -------------------
    float anchorX = charRect.x + charRect.w / 2.0f;
    float anchorY = charRect.y + charRect.h / 2.0f;
    float baseR = 70.0f;
    float maxAngle = 60.0f * (PI / 180.0f); // ±60 degrees
    float period = 2000.0f;
    float omega = 2 * PI / (period / 1000.0f);

    HookState hookState = OSCILLATING;
    Uint32 referenceTime = SDL_GetTicks();
    float currentAngle = 0.0f, storedAngle = 0.0f, currentR = baseR;
    float pullSpeed = 200.0f;
    float phaseOffset = 0.0f;

    // Scale hook.
    float scaleFactor = 0.05f;
    int hookW = (int)(596 * scaleFactor);
    int hookH = (int)(905 * scaleFactor);
    float hookX = anchorX, hookY = anchorY + baseR;
    SDL_Point hookPivot = {(int)(425 * scaleFactor + 0.5f), (int)(170 * scaleFactor + 0.5f)};

    SDL_Rect hookCollision;
    hookCollision.w = 20;
    hookCollision.h = 20;

    int score = 0;
    bool isPullingRock = false;
    int pulledGoldIndex = -1, pulledRockIndex = -1;
    
    // Variables for dinamo activation.
    float dinamoMoveTimeRemaining = 0.0f;
    float explosionTimeRemaining = 0.0f;
    float explosionX = 0.0f, explosionY = 0.0f; // where explosion is shown

    // New: number of available dinamos for testing.
    int availableDinamos = 10;

    // New: game timer set to 60 seconds.
    float gameTimer = 60.0f;

    bool quit = false;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    
    // ------------------- Main Game Loop -------------------
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            // When DOWN is pressed, if hook is oscillating, start pulling down.
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_DOWN) {
                    if (hookState == OSCILLATING) {
                        hookState = PULLING_DOWN;
                        storedAngle = currentAngle;
                    }
                }
                // NEW: When UP is pressed while an object is being pulled up...
                if (event.key.keysym.sym == SDLK_UP) {
                    // Only activate dinamo if available.
                    if (hookState == PULLING_GOLD && availableDinamos > 0) {
                        if ((!isPullingRock && pulledGoldIndex != -1) ||
                            (isPullingRock && pulledRockIndex != -1)) {
                            hookState = DINAMO_MOVING;
                            dinamoMoveTimeRemaining = 0.05f;
                            explosionX = hookX;
                            explosionY = hookY;
                            // Immediately remove the pulled object.
                            if (!isPullingRock && pulledGoldIndex != -1) {
                                golds[pulledGoldIndex].active = false;
                            } else if (isPullingRock && pulledRockIndex != -1) {
                                rocks[pulledRockIndex].active = false;
                            }
                            // Remove one dinamo (the rightmost one).
                            availableDinamos--;
                        }
                    }
                }
            }
        }

        Uint32 currentTimeTicks = SDL_GetTicks();
        float deltaTime = (currentTimeTicks - lastTime) / 1000.0f;
        lastTime = currentTimeTicks;

        // Update game timer.
        gameTimer -= deltaTime;
        if (gameTimer <= 0) {
            quit = true;
        }

        // Update anchor (character is static here).
        anchorX = charRect.x + charRect.w / 2.0f;
        anchorY = charRect.y + charRect.h / 2.0f;

        // ------------------- Update Hook Based on State -------------------
        switch (hookState) {
            case OSCILLATING: {
                float t = (currentTimeTicks - referenceTime) / 1000.0f;
                currentAngle = maxAngle * sin(omega * t + phaseOffset);
                currentR = baseR;
                hookX = anchorX + currentR * sin(currentAngle);
                hookY = anchorY + currentR * cos(currentAngle);
                break;
            }
            case PULLING_DOWN: {
                currentR += pullSpeed * deltaTime;
                hookX = anchorX + currentR * sin(storedAngle);
                hookY = anchorY + currentR * cos(storedAngle);
                if (hookY + hookH/2 >= 600 || hookX - hookW/2 <= 0 || hookX + hookW/2 >= 800) {
                    hookState = ROLLING_BACK;
                }
                break;
            }
            case ROLLING_BACK: {
                currentR -= pullSpeed * deltaTime;
                hookX = anchorX + currentR * sin(storedAngle);
                hookY = anchorY + currentR * cos(storedAngle);
                if (currentR <= baseR + 1.0f) {
                    currentR = baseR;
                    phaseOffset = asin(storedAngle / maxAngle);
                    referenceTime = currentTimeTicks;
                    hookState = OSCILLATING;
                }
                break;
            }
            case PULLING_GOLD: {
                float retractSpeed = isPullingRock ? pullSpeed * 0.5f : pullSpeed;
                currentR -= retractSpeed * deltaTime;
                hookX = anchorX + currentR * sin(storedAngle);
                hookY = anchorY + currentR * cos(storedAngle);
                if (!isPullingRock) {
                    for (int i = 0; i < NUM_GOLDS; i++) {
                        if (golds[i].active && SDL_HasIntersection(&hookCollision, &golds[i].rect)) {
                            pulledGoldIndex = i;
                            golds[i].rect.x = (int)hookX - golds[i].rect.w / 2;
                            golds[i].rect.y = (int)hookY - golds[i].rect.h / 2;
                            break;
                        }
                    }
                } else {
                    for (int i = 0; i < NUM_ROCKS; i++) {
                        if (rocks[i].active && SDL_HasIntersection(&hookCollision, &rocks[i].rect)) {
                            pulledRockIndex = i;
                            rocks[i].rect.x = (int)hookX - rocks[i].rect.w / 2;
                            rocks[i].rect.y = (int)hookY - rocks[i].rect.h / 2;
                            break;
                        }
                    }
                }
                if (currentR <= baseR + 1.0f) {
                    currentR = baseR;
                    hookState = OSCILLATING;
                    phaseOffset = asin(storedAngle / maxAngle);
                    referenceTime = currentTimeTicks;
                    if (isPullingRock && pulledRockIndex != -1) {
                        score += (rocks[pulledRockIndex].type == ROCK_SMALL) ? 10 : 20;
                        rocks[pulledRockIndex].active = false;
                        pulledRockIndex = -1;
                    } else if (!isPullingRock && pulledGoldIndex != -1) {
                        if (golds[pulledGoldIndex].type == GOLD_SMALL)
                            score += 50;
                        else if (golds[pulledGoldIndex].type == GOLD_MEDIUM)
                            score += 100;
                        else
                            score += 200;
                        golds[pulledGoldIndex].active = false;
                        pulledGoldIndex = -1;
                    }
                    printf("Score: %d\n", score);
                }
                break;
            }
            case DINAMO_MOVING: {
                dinamoMoveTimeRemaining -= deltaTime;
                if (dinamoMoveTimeRemaining <= 0) {
                    hookState = DINAMO_EXPLOSION;
                    explosionTimeRemaining = 0.2f;
                }
                break;
            }
            case DINAMO_EXPLOSION: {
                explosionTimeRemaining -= deltaTime;
                if (explosionTimeRemaining <= 0) {
                    currentR = baseR;
                    phaseOffset = asin(storedAngle / maxAngle);
                    referenceTime = currentTimeTicks;
                    hookState = OSCILLATING;
                    pulledGoldIndex = -1;
                    pulledRockIndex = -1;
                }
                break;
            }
        }

        // Build hook rectangle and update collision box.
        SDL_Rect hookRect;
        hookRect.x = (int)hookX - hookW / 2;
        hookRect.y = (int)hookY - hookH / 2;
        hookRect.w = hookW;
        hookRect.h = hookH;
        hookCollision.x = (int)hookX - hookCollision.w / 2;
        hookCollision.y = (int)hookY - hookCollision.h / 2;

        if (hookState == PULLING_DOWN) {
            for (int i = 0; i < NUM_GOLDS; i++) {
                if (golds[i].active && SDL_HasIntersection(&hookCollision, &golds[i].rect)) {
                    hookState = PULLING_GOLD;
                    isPullingRock = false;
                    pulledGoldIndex = i;
                    break;
                }
            }
            for (int i = 0; i < NUM_ROCKS; i++) {
                if (rocks[i].active && SDL_HasIntersection(&hookCollision, &rocks[i].rect)) {
                    hookState = PULLING_GOLD;
                    isPullingRock = true;
                    pulledRockIndex = i;
                    break;
                }
            }
        }

        // ------------------- Rendering -------------------
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        for (int i = 0; i < NUM_GOLDS; i++) {
            if (golds[i].active)
                SDL_RenderCopy(renderer, goldTexture, NULL, &golds[i].rect);
        }
        for (int i = 0; i < NUM_ROCKS; i++) {
            if (rocks[i].active)
                SDL_RenderCopy(renderer, rockTexture, NULL, &rocks[i].rect);
        }
        SDL_RenderCopy(renderer, charTexture, NULL, &charRect);

        float angleDeg = -(currentAngle * 180.0f / PI);
        if (hookState == DINAMO_MOVING) {
            SDL_Rect dinamoRect = hookRect;
            SDL_RenderCopyEx(renderer, dinamoTexture, NULL, &dinamoRect, angleDeg, &hookPivot, SDL_FLIP_NONE);
        }
        else if (hookState == DINAMO_EXPLOSION) {
            SDL_Rect explosionRect;
            explosionRect.x = (int)explosionX - hookW/2;
            explosionRect.y = (int)explosionY - hookH/2;
            explosionRect.w = 100;
            explosionRect.h = 100;
            SDL_RenderCopy(renderer, implodeTexture, NULL, &explosionRect);
        }
        else {
            SDL_RenderCopyEx(renderer, hookTexture, NULL, &hookRect, angleDeg, &hookPivot, SDL_FLIP_NONE);
        }

        int hookPivotScreenX = hookRect.x + hookPivot.x;
        int hookPivotScreenY = hookRect.y + hookPivot.y;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, (int)anchorX, (int)anchorY, hookPivotScreenX, hookPivotScreenY);

        // Render score text.
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

        // NEW: Render the available dinamos (50x50 each) arranged to the right of the character.
        for (int i = 0; i < availableDinamos; i++) {
            SDL_Rect dRect = { charRect.x + charRect.w + i * 50, 50, 50, 50 };
            SDL_RenderCopy(renderer, dinamoTexture, NULL, &dRect);
        }
        
        // NEW: Render the game timer (top left).
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

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // ------------------- Cleanup -------------------
    SDL_DestroyTexture(hookTexture);
    SDL_DestroyTexture(rockTexture);
    SDL_DestroyTexture(goldTexture);
    SDL_DestroyTexture(charTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(dinamoTexture);
    SDL_DestroyTexture(implodeTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}

// ---------------------------------------------------------------------
// runMenu: Show a menu with daovang.png as background.
//          User clicks on the gold portion to start the game.
//          Two extra buttons: "Control" and "High Scores".
//          Returns 0 if "Start Game" was selected (click on gold).
//          Returns 1 if the user closes/quits from menu.
// ---------------------------------------------------------------------
int runMenu(SDL_Renderer* renderer, TTF_Font* font)
{
    SDL_Surface* menuBGSurface = IMG_Load("daovang.png");
    if (!menuBGSurface) {
        printf("Error loading daovang.png: %s\n", IMG_GetError());
        return 1;
    }
    SDL_Texture* menuBGTexture = SDL_CreateTextureFromSurface(renderer, menuBGSurface);
    SDL_FreeSurface(menuBGSurface);

    SDL_Rect goldRect = { 50, 100, 200, 200 };
    SDL_Rect controlRect = { 50, 350, 200, 50 };
    SDL_Rect scoresRect  = { 50, 420, 200, 50 };

    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyTexture(menuBGTexture);
                return 1;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                if (mx >= goldRect.x && mx <= goldRect.x + goldRect.w &&
                    my >= goldRect.y && my <= goldRect.y + goldRect.h) {
                    SDL_DestroyTexture(menuBGTexture);
                    return 0;
                }
                if (mx >= controlRect.x && mx <= controlRect.x + controlRect.w &&
                    my >= controlRect.y && my <= controlRect.y + controlRect.h) {
                    showControls(renderer, font);
                }
                if (mx >= scoresRect.x && mx <= scoresRect.x + scoresRect.w &&
                    my >= scoresRect.y && my <= scoresRect.y + scoresRect.h) {
                    showHighScores(renderer, font);
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuBGTexture, NULL, NULL);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
        SDL_RenderFillRect(renderer, &controlRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
        SDL_RenderFillRect(renderer, &scoresRect);

        SDL_Color white = {255, 255, 255, 255};

        SDL_Surface* bgnSurf = TTF_RenderText_Blended(font, "Begin", white);
        if (bgnSurf) {
            SDL_Texture* bgnText = SDL_CreateTextureFromSurface(renderer, bgnSurf);
            SDL_FreeSurface(bgnSurf);
            SDL_Rect bgnTextRect = { goldRect.x, goldRect.y, 0, 0 };
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
            SDL_Rect ctrlTextRect = { controlRect.x, controlRect.y, 0, 0 };
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
            SDL_Rect scoresTextRect = { scoresRect.x, scoresRect.y, 0, 0 };
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

// ---------------------------------------------------------------------
// showControls: Display an overlay with control instructions.
// Press ESC to go back.
// ---------------------------------------------------------------------
void showControls(SDL_Renderer* renderer, TTF_Font* font)
{
    bool done = false;
    SDL_Event e;
    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                done = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderClear(renderer);
        SDL_Color white = {255, 255, 255, 255};
        const char* line1 = "Controls:";
        const char* line2 = "Up button = Dynamite";
        const char* line3 = "Down button = Lower Mining Crank";
        const char* line4 = "(Press ESC to go back)";
        auto renderLine = [&](const char* text, int yOffset) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, text, white);
            if (!surf) return;
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            SDL_Rect rect = {0, 0, 0, 0};
            SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);
            rect.x = (800 - rect.w) / 2;
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

// ---------------------------------------------------------------------
// showHighScores: Placeholder for high scores screen.
// Press ESC to go back.
// ---------------------------------------------------------------------
void showHighScores(SDL_Renderer* renderer, TTF_Font* font)
{
    bool done = false;
    SDL_Event e;
    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                done = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderClear(renderer);
        SDL_Color white = {255, 255, 255, 255};
        const char* msg = "High Scores - Not implemented yet.\n(Press ESC to go back)";
        SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, msg, white, 600);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            SDL_Rect rect = {0, 0, 0, 0};
            SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);
            rect.x = (800 - rect.w) / 2;
            rect.y = (600 - rect.h) / 2;
            SDL_RenderCopy(renderer, tex, NULL, &rect);
            SDL_DestroyTexture(tex);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
