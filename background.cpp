#include <SDL.h>
#include <SDL_image.h>
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
    PULLING_GOLD
} HookState;

int main(int argc, char* argv[]) {
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // Create SDL window.
    SDL_Window* window = SDL_CreateWindow("SDL Game with Hook",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create renderer.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL_image.
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // --- Load Images ---
    // Background.
    SDL_Surface* bgSurface = IMG_Load("background.png");
    if (!bgSurface) {
        printf("Unable to load background.png! SDL_image Error: %s\n", IMG_GetError());
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    if (!bgTexture) {
        printf("Unable to create background texture! SDL_Error: %s\n", SDL_GetError());
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    
    // Character (assumed width 200 for centering).
    SDL_Surface* charSurface = IMG_Load("character.png");
    if (!charSurface) {
        printf("Unable to load character.png! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, charSurface);
    SDL_FreeSurface(charSurface);
    if (!charTexture) {
        printf("Unable to create character texture! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    
    // Gold object.
    SDL_Surface* goldSurface = IMG_Load("gold.png");
    if (!goldSurface) {
        printf("Unable to load gold.png! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(charTexture); SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    SDL_Texture* goldTexture = SDL_CreateTextureFromSurface(renderer, goldSurface);
    SDL_FreeSurface(goldSurface);
    if (!goldTexture) {
        printf("Unable to create gold texture! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(charTexture); SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    
    // Hook.
    SDL_Surface* hookSurface = IMG_Load("hook.png");
    if (!hookSurface) {
        printf("Unable to load hook.png! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(goldTexture); SDL_DestroyTexture(charTexture);
        SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    SDL_Texture* hookTexture = SDL_CreateTextureFromSurface(renderer, hookSurface);
    SDL_FreeSurface(hookSurface);
    if (!hookTexture) {
        printf("Unable to create hook texture! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(goldTexture); SDL_DestroyTexture(charTexture);
        SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    
    // Rock (optional object).
    SDL_Surface* rockSurface = IMG_Load("rock.png");
    if (!rockSurface) {
        printf("Unable to load rock.png! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(hookTexture); SDL_DestroyTexture(goldTexture);
        SDL_DestroyTexture(charTexture); SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    SDL_Texture* rockTexture = SDL_CreateTextureFromSurface(renderer, rockSurface);
    SDL_FreeSurface(rockSurface);
    if (!rockTexture) {
        printf("Unable to create rock texture! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(hookTexture); SDL_DestroyTexture(goldTexture);
        SDL_DestroyTexture(charTexture); SDL_DestroyTexture(bgTexture);
        IMG_Quit(); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    
    // --- Define Destination Rectangles ---
    // Center the character horizontally in an 800px window.
    SDL_Rect charRect = {300, 100, 200, 100};  // x = (800-200)/2 = 300.
    SDL_Rect goldRect = {200, 400, 50, 50};
    SDL_Rect rockRect = {500, 400, 50, 50};
    bool rockActive = true;
    bool goldActive = true;
    bool isPullingRock = false;
    
    // --- Hook Parameters ---
    // The hook's anchor is the character's center.
    float anchorX = charRect.x + charRect.w / 2.0f;
    float anchorY = charRect.y + charRect.h / 2.0f;
    
    float baseR = 70.0f;                       // Base distance from anchor when oscillating.
    float maxAngle = 60.0f * (PI / 180.0f);      // ±60° (in radians).
    float period = 2000.0f;                      // Oscillation period in ms.
    float omega = 2 * PI / (period / 1000.0f);
    
    HookState hookState = OSCILLATING;
    
    // Variables to help continue the oscillation smoothly.
    float oscillationStartTimeOffset = 0.0f; // Offset for global time to compute phase.
    float storedOscillationPhase = 0.0f;     // Store the phase (elapsed time) when leaving oscillation.
    
    float currentAngle = 0.0f;
    float storedAngle = 0.0f;
    float currentR = baseR;
    float pullSpeed = 200.0f;
    
    // --- Use original hook scale ---
    // Original scale factor is 0.07.
    float scaleFactor = 0.05f;  // Using the original scale factor.
    int hookW = (int)(596 * scaleFactor);
    int hookH = (int)(905 * scaleFactor);
    // Adjust the pivot accordingly.
    // The original true center is (425,170) in the hook image.
    SDL_Point hookPivot = { (int)(425 * scaleFactor + 0.5f), (int)(170 * scaleFactor + 0.5f) };
    
    float hookX = anchorX, hookY = anchorY + baseR;
    
    // A smaller collision box.
    SDL_Rect hookCollision;
    hookCollision.w = 20;  // adjust as needed
    hookCollision.h = 20;
    
    bool quit = false;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    
    // Main game loop.
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_DOWN) {
                    if (hookState == OSCILLATING) {
                        hookState = PULLING_DOWN;
                        storedAngle = currentAngle;
                        storedOscillationPhase = SDL_GetTicks() - oscillationStartTimeOffset;
                    }
                }
            }
        }
        
        Uint32 currentTimeTicks = SDL_GetTicks();
        float deltaTime = (currentTimeTicks - lastTime) / 1000.0f;
        lastTime = currentTimeTicks;
        
        // Update anchor in case character moves.
        anchorX = charRect.x + charRect.w / 2.0f;
        anchorY = charRect.y + charRect.h / 2.0f;
        
        switch (hookState) {
            case OSCILLATING: {
                // Continue oscillation using a time offset.
                currentAngle = maxAngle * sin(omega * ((currentTimeTicks - oscillationStartTimeOffset) / 1000.0f));
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
                if (currentR <= baseR) {
                    currentR = baseR;
                    hookState = OSCILLATING;
                    // Resume oscillation from the stored phase.
                    oscillationStartTimeOffset = currentTimeTicks - storedOscillationPhase;
                }
                break;
            }
            case PULLING_GOLD: {
                float retractSpeed = isPullingRock ? pullSpeed * 0.5f : pullSpeed;
                currentR -= retractSpeed * deltaTime;
                hookX = anchorX + currentR * sin(storedAngle);
                hookY = anchorY + currentR * cos(storedAngle);
                if (!isPullingRock) {
                    goldRect.x = (int)hookX - goldRect.w / 2;
                    goldRect.y = (int)hookY - goldRect.h / 2;
                } else {
                    rockRect.x = (int)hookX - rockRect.w / 2;
                    rockRect.y = (int)hookY - rockRect.h / 2;
                }
                if (currentR <= baseR) {
                    currentR = baseR;
                    hookState = OSCILLATING;
                    oscillationStartTimeOffset = currentTimeTicks - storedOscillationPhase;
                    if (isPullingRock)
                        rockActive = false;
                    else
                        goldActive = false;
                }
                break;
            }
        }
        
        // Build hook rectangle.
        SDL_Rect hookRect;
        hookRect.x = (int)hookX - hookW / 2;
        hookRect.y = (int)hookY - hookH / 2;
        hookRect.w = hookW;
        hookRect.h = hookH;
        
        // Build collision rectangle.
        hookCollision.x = (int)hookX - hookCollision.w / 2;
        hookCollision.y = (int)hookY - hookCollision.h / 2;
        
        // Check for collisions while pulling down.
        if (goldActive && hookState == PULLING_DOWN && SDL_HasIntersection(&hookCollision, &goldRect)) {
            hookState = PULLING_GOLD;
            isPullingRock = false;
        }
        if (rockActive && hookState == PULLING_DOWN && SDL_HasIntersection(&hookCollision, &rockRect)) {
            hookState = PULLING_GOLD;
            isPullingRock = true;
        }
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        if (goldActive)
            SDL_RenderCopy(renderer, goldTexture, NULL, &goldRect);
        if (rockActive)
            SDL_RenderCopy(renderer, rockTexture, NULL, &rockRect);
        SDL_RenderCopy(renderer, charTexture, NULL, &charRect);
        
        // Compute rotation angle for hook drawing (negative to match the string direction).
        float angleDeg = -(currentAngle * 180.0f / PI);
        SDL_RenderCopyEx(renderer, hookTexture, NULL, &hookRect, angleDeg, &hookPivot, SDL_FLIP_NONE);
        
        // Draw the string from character's anchor to the hook's true center.
        int hookPivotScreenX = hookRect.x + hookPivot.x;
        int hookPivotScreenY = hookRect.y + hookPivot.y;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, (int)anchorX, (int)anchorY, hookPivotScreenX, hookPivotScreenY);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    // Cleanup.
    SDL_DestroyTexture(hookTexture);
    SDL_DestroyTexture(rockTexture);
    SDL_DestroyTexture(goldTexture);
    SDL_DestroyTexture(charTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
