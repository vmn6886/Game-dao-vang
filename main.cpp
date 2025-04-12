#include <SDL.h> // Include SDL main header for graphics, events, etc.
#include <SDL_image.h> // Include SDL_image for loading image files
#include <SDL_ttf.h> // Include SDL_ttf for using TrueType fonts
#include <SDL_mixer.h> // Include SDL_mixer for sound and music support
#include <stdio.h> // Include standard I/O functions
#include <stdbool.h> // Include boolean type support
#include <math.h> // Include math functions such as sin and cos
#include <stdlib.h> // Include standard library functions such as rand, srand
#include <time.h> // Include time functions for seeding random generator
#include "objects.h" // Include custom header that defines GoldObject and RockObject
#define PI 3.14159265358979323846 // Define a constant for PI
typedef enum { OSCILLATING, PULLING_DOWN, ROLLING_BACK, PULLING_GOLD, dynamite_MOVING, dynamite_EXPLOSION } HookState; // Define the hook states used in the game
#define NUM_GOLDS 11 // Total number of gold objects
#define NUM_ROCKS 4 // Total number of rock objects
int runMenu(SDL_Renderer* renderer, TTF_Font* font); // Forward declaration for main menu function
void showControls(SDL_Renderer* renderer, TTF_Font* font); // Forward declaration for controls overlay function
void showHighScores(SDL_Renderer* renderer, TTF_Font* font); // Forward declaration for high scores function
void showTargetScreen(SDL_Renderer* renderer, TTF_Font* font48, SDL_Texture* targetTexture, Mix_Music* targetMusic, int neededPoints); // Forward declaration for target screen function
int main(int argc, char* argv[]) { // Main function entry point
    srand((unsigned int)time(NULL)); // Seed the random number generator with current time
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { // Initialize SDL video subsystem and check for error
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError()); // Print error message if initialization fails
        return 1; // Exit program with error
    }
    if (TTF_Init() == -1) { // Initialize SDL_ttf for fonts and check for error
        printf("TTF_Init: %s\n", TTF_GetError()); // Print error message if TTF fails to initialize
        SDL_Quit(); // Clean up SDL
        return 1; // Exit program with error
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { // Initialize SDL_mixer for audio playback
        printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError()); // Print error if mixer fails to initialize
        return 1; // Exit program with error
    }
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Load 24pt font for in-game text
    if (!font) { // Check if font loading fails
        printf("Failed to load font (24pt): %s\n", TTF_GetError()); // Print error message
        return 1; // Exit if font is not loaded
    }
    TTF_Font* font48 = TTF_OpenFont("arial.ttf", 48); // Load 48pt font for large text (e.g. target screen)
    if (!font48) { // Check if 48pt font loading fails
        printf("Failed to load font (48pt): %s\n", TTF_GetError()); // Print error message
        return 1; // Exit if font is not loaded
    }
    SDL_Window* window = SDL_CreateWindow("Đào vàng", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1366, 768, SDL_WINDOW_SHOWN); // Create a window with size 1366x768 and title "Đào vàng"
    if (!window) { // If window creation fails
        printf("Window error: %s\n", SDL_GetError()); // Print error message
        return 1; // Exit program if window cannot be created
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Create a hardware-accelerated renderer
    if (!renderer) { // Check for renderer creation failure
        printf("Renderer error: %s\n", SDL_GetError()); // Print error message
        SDL_DestroyWindow(window); // Destroy window if renderer creation fails
        return 1; // Exit program
    }
    int imgFlags = IMG_INIT_PNG; // Set flag to load PNG images
    if (!(IMG_Init(imgFlags) & imgFlags)) { // Initialize SDL_image and check for PNG support
        printf("SDL_image error: %s\n", IMG_GetError()); // Print error message if image library fails
        SDL_DestroyRenderer(renderer); // Clean up renderer
        SDL_DestroyWindow(window); // Clean up window
        return 1; // Exit program
    }
    SDL_Surface* bgSurface = IMG_Load("background.png"); // Load background image from file
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface); // Create texture from background surface
    SDL_FreeSurface(bgSurface); // Free background surface after texture creation
    SDL_Surface* charSurface = IMG_Load("character.png"); // Load character image from file
    SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, charSurface); // Create texture for character
    SDL_FreeSurface(charSurface); // Free character surface
    SDL_Surface* goldSurface = IMG_Load("gold.png"); // Load gold image from file
    SDL_Texture* goldTexture = SDL_CreateTextureFromSurface(renderer, goldSurface); // Create texture for gold objects
    SDL_FreeSurface(goldSurface); // Free gold surface
    SDL_Surface* hookSurface = IMG_Load("hook.png"); // Load hook image from file
    SDL_Texture* hookTexture = SDL_CreateTextureFromSurface(renderer, hookSurface); // Create texture for hook
    SDL_FreeSurface(hookSurface); // Free hook surface
    SDL_Surface* rockSurface = IMG_Load("rock.png"); // Load rock image from file
    SDL_Texture* rockTexture = SDL_CreateTextureFromSurface(renderer, rockSurface); // Create texture for rock objects
    SDL_FreeSurface(rockSurface); // Free rock surface
    SDL_Surface* dynamiteSurface = IMG_Load("dynamite.png"); // Load dynamite image
    if (!dynamiteSurface) // Check if dynamite image loaded successfully
        printf("Error loading dynamite.png: %s\n", IMG_GetError()); // Print error message if dynamite fails
    SDL_Texture* dynamiteTexture = SDL_CreateTextureFromSurface(renderer, dynamiteSurface); // Create texture for dynamite
    SDL_FreeSurface(dynamiteSurface); // Free dynamite surface
    SDL_Surface* implodeSurface = IMG_Load("explosion.png"); // Load explosion image
    if (!implodeSurface) // Check if explosion image loaded successfully
        printf("Error loading explosion.png: %s\n", IMG_GetError()); // Print error if explosion image fails
    SDL_Texture* implodeTexture = SDL_CreateTextureFromSurface(renderer, implodeSurface); // Create texture for explosion effect
    SDL_FreeSurface(implodeSurface); // Free explosion surface
    SDL_Surface* mysbagSurface = IMG_Load("mysbag.png"); // Load mystery bag image
    if (!mysbagSurface) // Check if mystery bag image loaded successfully
        printf("Error loading mysbag.png: %s\n", IMG_GetError()); // Print error if mystery bag fails
    SDL_Texture* mysbagTexture = SDL_CreateTextureFromSurface(renderer, mysbagSurface); // Create texture for mystery bag
    SDL_FreeSurface(mysbagSurface); // Free mystery bag surface
    SDL_Surface* successSurface = IMG_Load("success.png"); // Load success image
    if (!successSurface) // Check if success image loaded successfully
        printf("Error loading success.png: %s\n", IMG_GetError()); // Print error if success image fails
    SDL_Texture* successTexture = SDL_CreateTextureFromSurface(renderer, successSurface); // Create texture for success screen
    SDL_FreeSurface(successSurface); // Free success surface
    SDL_Surface* failureSurface = IMG_Load("failure.png"); // Load failure image
    if (!failureSurface) // Check if failure image loaded successfully
        printf("Error loading failure.png: %s\n", IMG_GetError()); // Print error if failure image fails
    SDL_Texture* failureTexture = SDL_CreateTextureFromSurface(renderer, failureSurface); // Create texture for failure screen
    SDL_FreeSurface(failureSurface); // Free failure surface
    SDL_Surface* targetSurface = IMG_Load("Target.png"); // Load target screen image
    if (!targetSurface) // Check if target image loaded successfully
        printf("Error loading Target.png: %s\n", IMG_GetError()); // Print error if target image fails
    SDL_Texture* targetTexture = SDL_CreateTextureFromSurface(renderer, targetSurface); // Create texture for target screen
    SDL_FreeSurface(targetSurface); // Free target surface
    Mix_Music* targetMusic = Mix_LoadMUS("target.mp3"); // Load target music/sound
    if (!targetMusic) // Check if target music loaded successfully
        printf("Error loading target.mp3: %s\n", Mix_GetError()); // Print error if target music fails
    // Note: The target screen call has been removed from here to avoid appearing before the main menu.
    bool exitProgram = false; // Flag to control overall program loop (for multiple game sessions)
    while (!exitProgram) { // Outer loop for game sessions
        int menuResult = runMenu(renderer, font); // Display the main menu and wait for user selection
        if (menuResult == 1) { // If the menu returns a quit signal
            exitProgram = true; // Set exit flag to true
            break; // Break out of the outer loop
        }
        showTargetScreen(renderer, font48, targetTexture, targetMusic, 400);
        SDL_Rect charRect = {583, 90, 200, 100}; // Define character rectangle (centered with feet near 170px from top)
        GoldObject golds[NUM_GOLDS]; // Declare array of gold objects
        golds[0].type = GOLD_SMALL;  golds[0].rect = (SDL_Rect){50,300,20,20};   golds[0].active = true; // Initialize gold object 0 (left side)
        golds[1].type = GOLD_SMALL;  golds[1].rect = (SDL_Rect){1250,320,20,20};  golds[1].active = true; // Initialize gold object 1 (right side)
        golds[2].type = GOLD_SMALL;  golds[2].rect = (SDL_Rect){350,340,20,20};   golds[2].active = true; // Gold object 2 near left-middle
        golds[3].type = GOLD_SMALL;  golds[3].rect = (SDL_Rect){600,360,20,20};   golds[3].active = true; // Gold object 3 (center)
        golds[4].type = GOLD_SMALL;  golds[4].rect = (SDL_Rect){900,280,20,20};   golds[4].active = true; // Gold object 4 (right)
        golds[5].type = GOLD_MEDIUM; golds[5].rect = (SDL_Rect){500,520,30,30};   golds[5].active = true; // Medium gold object 5
        golds[6].type = GOLD_MEDIUM; golds[6].rect = (SDL_Rect){1150,640,30,30};  golds[6].active = true; // Medium gold object 6 on right side
        golds[7].type = GOLD_MEDIUM; golds[7].rect = (SDL_Rect){800,700,30,30};   golds[7].active = true; // Medium gold object 7 (center-bottom)
        golds[8].type = GOLD_BIG;    golds[8].rect = (SDL_Rect){450,600,60,60};   golds[8].active = true; // Big gold object 8 (left-middle)
        golds[9].type = GOLD_BIG;    golds[9].rect = (SDL_Rect){1000,690,60,60};  golds[9].active = true; // Big gold object 9 (right-middle)
        golds[10].type = GOLD_MYSTERY; golds[10].rect = (SDL_Rect){400,450,40,40};  golds[10].active = true; // Mystery gold object 10
        RockObject rocks[NUM_ROCKS]; // Declare array of rock objects
        rocks[0].type = ROCK_BIG;   rocks[0].rect = (SDL_Rect){250,370,50,50};   rocks[0].active = true; // Big rock object 0 (left)
        rocks[1].type = ROCK_BIG;   rocks[1].rect = (SDL_Rect){550,380,50,50};   rocks[1].active = true; // Big rock object 1 (center-left)
        rocks[2].type = ROCK_SMALL; rocks[2].rect = (SDL_Rect){900,320,30,30};   rocks[2].active = true; // Small rock object 2 (right)
        rocks[3].type = ROCK_SMALL; rocks[3].rect = (SDL_Rect){1050,340,30,30};  rocks[3].active = true; // Small rock object 3 (far right)
        float anchorX = charRect.x + charRect.w / 2.0f; // Compute hook's anchor X from character center
        float anchorY = charRect.y + charRect.h / 2.0f; // Compute hook's anchor Y from character center
        float baseR = 70.0f; // Set base hook length
        float maxAngle = 75.0f * (PI / 180.0f); // Set maximum oscillation angle in radians
        float period = 2000.0f; // Oscillation period (ms) – not directly used here
        float omega = 2*PI/(period/1000.0f); // Calculate angular velocity from period
        HookState hookState = OSCILLATING; // Start hook state at OSCILLATING
        Uint32 refTime = SDL_GetTicks(); // Record reference time for oscillation
        float currentAngle = 0.0f, storedAngle = 0.0f, currentR = baseR; // Initialize current angle, stored angle, and hook radius
        float pullSpeed = 200.0f; // Set hook retraction speed
        float droppingSpeed = 1000.0f; // Set hook extension speed
        float phaseOffset = 0.0f; // Initialize oscillation phase offset
        float hookX, hookY; // Variables to store current hook X and Y
        float scaleFactor = 0.05f; // Set scale factor for hook image
        int hookW = (int)(928 * scaleFactor); // Calculate hook image width from original dimensions
        int hookH = (int)(665 * scaleFactor); // Calculate hook image height from original dimensions
        hookX = anchorX; hookY = anchorY + baseR; // Initialize hook position at character anchor plus base radius vertically
        SDL_Point hookPivot = {(int)(465 * scaleFactor + 0.5f), (int)(77 * scaleFactor + 0.5f)}; // Set pivot where the red string attaches on the hook image
        SDL_Rect hookCollision; // Declare a collision rectangle for the hook
        hookCollision.w = 20; hookCollision.h = 20; // Set hook collision rectangle dimensions
        int score = 0; // Initialize game score
        bool isPullingRock = false; // Flag for whether the hook is pulling a rock
        int pulledGoldIndex = -1, pulledRockIndex = -1; // Indexes for the currently pulled gold or rock object
        float dynamiteMoveTimeRemaining = 0.0f; // Timer for dynamite movement
        float explosionTimeRemaining = 0.0f; // Timer for explosion effect duration
        float explosionX = 0.0f, explosionY = 0.0f; // Coordinates for explosion effect
        int availabledynamites = 0; // Count of available dynamites
        float gameTimer = 60.0f; // Initialize game timer to 60 seconds
        bool timeUp = false; // Flag to indicate whether game time has expired
        bool quitSession = false; // Flag to exit the current game session loop
        Uint32 lastTime = SDL_GetTicks(); // Record last tick time for delta time calculation
        while (!quitSession) { // Begin game session loop
            SDL_Event event; // Declare event variable
            while (SDL_PollEvent(&event)) { // Poll for events
                if (event.type == SDL_QUIT) { // If quit event is received
                    SDL_DestroyRenderer(renderer); // Destroy renderer
                    SDL_DestroyWindow(window); // Destroy window
                    TTF_CloseFont(font); // Close font
                    TTF_CloseFont(font48); // Close large font
                    Mix_CloseAudio(); // Close audio system
                    IMG_Quit(); // Quit SDL_image
                    TTF_Quit(); // Quit SDL_ttf
                    SDL_Quit(); // Quit SDL
                    exit(0); // Immediately exit the application
                }
                if (event.type == SDL_KEYDOWN) { // If a key is pressed
                    if (event.key.keysym.sym == SDLK_DOWN && !timeUp) { // If Down key is pressed and time is not up
                        if (hookState == OSCILLATING) { // If hook is in oscillating state
                            hookState = PULLING_DOWN; // Change state to PULLING_DOWN
                            storedAngle = currentAngle; // Save the current angle for subsequent pulling
                        }
                    }
                    if (event.key.keysym.sym == SDLK_UP && !timeUp) { // If Up key is pressed and time is not up
                        if (hookState == PULLING_GOLD && availabledynamites > 0) { // If hook is pulling an object and dynamites are available
                            if ((!isPullingRock && pulledGoldIndex != -1) || (isPullingRock && pulledRockIndex != -1)) { // If an object is actually being pulled
                                hookState = dynamite_MOVING; // Change state to dynamite_MOVING
                                dynamiteMoveTimeRemaining = 0.05f; // Set timer for dynamite movement
                                explosionX = hookX; // Record hook X for explosion effect
                                explosionY = hookY; // Record hook Y for explosion effect
                                if (!isPullingRock && pulledGoldIndex != -1) // If pulling gold
                                    golds[pulledGoldIndex].active = false; // Deactivate the gold object
                                else if (isPullingRock && pulledRockIndex != -1) // If pulling rock
                                    rocks[pulledRockIndex].active = false; // Deactivate the rock object
                                availabledynamites--; // Decrease dynamite count
                            }
                        }
                    }
                }
            }
            Uint32 currentTime = SDL_GetTicks(); // Get current tick count
            float deltaTime = (currentTime - lastTime) / 1000.0f; // Compute delta time in seconds
            lastTime = currentTime; // Update last tick time
            if (!timeUp) { // If game time remains
                gameTimer -= deltaTime; // Decrease the game timer
                if (gameTimer <= 0) { // If game timer reaches or drops below zero
                    gameTimer = 0; // Clamp game timer to zero
                    timeUp = true; // Set time-up flag
                    break; // Break out of the game session loop
                }
            }
            anchorX = charRect.x + charRect.w / 2.0f; // Recalculate the hook's anchor X from character
            anchorY = charRect.y + charRect.h / 2.0f; // Recalculate the hook's anchor Y from character
            if (!timeUp) { // If game is still running
                switch (hookState) { // Switch based on hook state
                    case OSCILLATING: { // When hook is oscillating
                        float t = (currentTime - refTime) / 1000.0f; // Compute elapsed time in seconds
                        currentAngle = maxAngle * sin(omega * t + phaseOffset); // Calculate oscillation angle
                        currentR = baseR; // Reset hook length to base
                        hookX = anchorX + currentR * sin(currentAngle); // Compute hook X based on oscillation
                        hookY = anchorY + currentR * cos(currentAngle); // Compute hook Y based on oscillation
                        break;
                    }
                    case PULLING_DOWN: { // When hook is being extended
                        currentR += droppingSpeed * deltaTime; // Increase hook length by droppingSpeed scaled by deltaTime
                        hookX = anchorX + currentR * sin(storedAngle); // Update hook X using stored angle
                        hookY = anchorY + currentR * cos(storedAngle); // Update hook Y using stored angle
                        if (hookY + hookH/2 >= 768 || hookX - hookW/2 <= 0 || hookX + hookW/2 >= 1366) // Check if hook touches boundaries
                            hookState = ROLLING_BACK; // Change state to ROLLING_BACK if boundary touched
                        break;
                    }
                    case ROLLING_BACK: { // When hook is retracting
                        currentR -= droppingSpeed * deltaTime; // Decrease hook length for retraction
                        hookX = anchorX + currentR * sin(storedAngle); // Update hook X during retraction
                        hookY = anchorY + currentR * cos(storedAngle); // Update hook Y during retraction
                        if (currentR <= baseR + 1.0f) { // If hook retracts near its base length
                            currentR = baseR; // Reset hook length to base
                            phaseOffset = asin(storedAngle / maxAngle); // Update phase offset for oscillation
                            refTime = currentTime; // Reset reference time
                            hookState = OSCILLATING; // Change state back to OSCILLATING
                        }
                        break;
                    }
                    case PULLING_GOLD: { // When hook has caught an object and is retracting
                        float retractSpeed = isPullingRock ? pullSpeed * 0.5f : pullSpeed; // Choose retract speed (slower for rocks)
                        currentR -= retractSpeed * deltaTime; // Decrease hook length by retract speed
                        hookX = anchorX + currentR * sin(storedAngle); // Update hook X during retraction
                        hookY = anchorY + currentR * cos(storedAngle); // Update hook Y during retraction
                        SDL_Rect hookRect; // Declare rectangle for rendering the hook
                        hookRect.x = (int)hookX - hookW/2; // Compute hook rectangle X position
                        hookRect.y = (int)hookY - hookH/2; // Compute hook rectangle Y position
                        hookRect.w = hookW; // Set hook rectangle width
                        hookRect.h = hookH; // Set hook rectangle height
                        int hookCenterX = hookRect.x + hookPivot.x; // Compute hook attachment point X using pivot
                        int hookCenterY = hookRect.y + hookPivot.y; // Compute hook attachment point Y using pivot
                        if (!isPullingRock && pulledGoldIndex != -1) { // If a gold object is being pulled
                            golds[pulledGoldIndex].rect.x = hookCenterX - golds[pulledGoldIndex].rect.w/2; // Center gold on hook
                            golds[pulledGoldIndex].rect.y = hookCenterY - golds[pulledGoldIndex].rect.h/2; // Center gold on hook
                        } else if (isPullingRock && pulledRockIndex != -1) { // Else if a rock is being pulled
                            rocks[pulledRockIndex].rect.x = hookCenterX - rocks[pulledRockIndex].rect.w/2; // Center rock on hook
                            rocks[pulledRockIndex].rect.y = hookCenterY - rocks[pulledRockIndex].rect.h/2; // Center rock on hook
                        }
                        if (currentR <= baseR + 1.0f) { // When hook retracts near its base
                            currentR = baseR; // Reset hook length to base
                            hookState = OSCILLATING; // Change state back to OSCILLATING
                            phaseOffset = asin(storedAngle / maxAngle); // Reset phase offset
                            refTime = currentTime; // Reset reference time
                            if (!isPullingRock && pulledGoldIndex != -1) { // If a gold object was pulled
                                if (golds[pulledGoldIndex].type == GOLD_MYSTERY) { // Check if it's mystery gold
                                    int r = rand() % 100; // Generate random value 0-99
                                    if (r < 30)
                                        availabledynamites++; // 30% chance to gain a dynamite
                                    else if (r < 90)
                                        score += 100; // 60% chance for 100 points
                                    else
                                        score += 250; // 10% chance for 250 points
                                } else if (golds[pulledGoldIndex].type == GOLD_SMALL)
                                    score += 50; // Small gold gives 50 points
                                else if (golds[pulledGoldIndex].type == GOLD_MEDIUM)
                                    score += 100; // Medium gold gives 100 points
                                else
                                    score += 200; // Big gold gives 200 points
                                golds[pulledGoldIndex].active = false; // Deactivate pulled gold
                                pulledGoldIndex = -1; // Reset index
                            } else if (isPullingRock && pulledRockIndex != -1) { // If a rock was pulled
                                score += (rocks[pulledRockIndex].type == ROCK_SMALL) ? 10 : 20; // Score based on rock size
                                rocks[pulledRockIndex].active = false; // Deactivate rock
                                pulledRockIndex = -1; // Reset index
                            }
                            printf("Score: %d\n", score); // Print current score
                        }
                        break;
                    }
                    case dynamite_MOVING: { // When dynamite is moving toward the pulled object
                        dynamiteMoveTimeRemaining -= deltaTime; // Decrease dynamite timer
                        if (dynamiteMoveTimeRemaining <= 0) { // If timer expires
                            hookState = dynamite_EXPLOSION; // Change state to explosion
                            explosionTimeRemaining = 0.2f; // Set explosion duration
                        }
                        break;
                    }
                    case dynamite_EXPLOSION: { // When explosion effect is active
                        explosionTimeRemaining -= deltaTime; // Decrease explosion timer
                        if (explosionTimeRemaining <= 0) { // If explosion timer expires
                            currentR = baseR; // Reset hook length
                            phaseOffset = asin(storedAngle / maxAngle); // Reset phase offset
                            refTime = currentTime; // Reset reference time
                            hookState = OSCILLATING; // Change state back to OSCILLATING
                            pulledGoldIndex = -1; // Reset pulled gold index
                            pulledRockIndex = -1; // Reset pulled rock index
                        }
                        break;
                    }
                }
            }
            SDL_Rect hookRect; // Declare rectangle for hook rendering
            hookRect.x = (int)hookX - hookW/2; // Compute hook rectangle X position
            hookRect.y = (int)hookY - hookH/2; // Compute hook rectangle Y position
            hookRect.w = hookW; // Set hook rectangle width
            hookRect.h = hookH; // Set hook rectangle height
            hookCollision.x = (int)hookX - hookCollision.w/2; // Compute collision rectangle X position
            hookCollision.y = (int)hookY - hookCollision.h/2; // Compute collision rectangle Y position
            int hookCenterX = hookRect.x + hookPivot.x; // Compute hook center X (attachment point)
            int hookCenterY = hookRect.y + hookPivot.y; // Compute hook center Y (attachment point)
            if (!timeUp && hookState == PULLING_DOWN) { // If hook is extending and game is still running
                float bestDist = 1e9f; // Initialize best distance to a very large number
                bool found = false; // Flag to indicate if a colliding object was found
                int bestIndex = -1; // Variable to store index of the closest colliding object
                bool bestIsRock = false; // Flag indicating the type of object (rock or gold)
                for (int i = 0; i < NUM_GOLDS; i++) { // Loop through all gold objects
                    if (golds[i].active && SDL_HasIntersection(&hookCollision, &golds[i].rect)) { // Check collision with gold
                        int objCenterX = golds[i].rect.x + golds[i].rect.w/2; // Compute gold center X
                        int objCenterY = golds[i].rect.y + golds[i].rect.h/2; // Compute gold center Y
                        float dx = (float)(objCenterX - hookCenterX); // Compute horizontal difference
                        float dy = (float)(objCenterY - hookCenterY); // Compute vertical difference
                        float dist = dx * dx + dy * dy; // Compute squared distance
                        if (dist < bestDist) { // If this object is closer than previous ones
                            bestDist = dist; // Update best distance
                            bestIndex = i; // Save its index
                            bestIsRock = false; // Mark as gold
                            found = true; // Set found flag
                        }
                    }
                }
                for (int i = 0; i < NUM_ROCKS; i++) { // Loop through all rock objects
                    if (rocks[i].active && SDL_HasIntersection(&hookCollision, &rocks[i].rect)) { // Check collision with rock
                        int objCenterX = rocks[i].rect.x + rocks[i].rect.w/2; // Compute rock center X
                        int objCenterY = rocks[i].rect.y + rocks[i].rect.h/2; // Compute rock center Y
                        float dx = (float)(objCenterX - hookCenterX); // Compute horizontal difference
                        float dy = (float)(objCenterY - hookCenterY); // Compute vertical difference
                        float dist = dx * dx + dy * dy; // Compute squared distance
                        if (dist < bestDist) { // If this rock is closer
                            bestDist = dist; // Update best distance
                            bestIndex = i; // Save rock index
                            bestIsRock = true; // Mark as rock
                            found = true; // Set found flag
                        }
                    }
                }
                if (found) { // If any colliding object was found
                    hookState = PULLING_GOLD; // Set hook state to PULLING_GOLD
                    if (bestIsRock) { // If the best object is a rock
                        isPullingRock = true; // Flag that a rock is being pulled
                        pulledRockIndex = bestIndex; // Save rock index
                    } else { // Otherwise, if it's gold
                        isPullingRock = false; // Flag that gold is being pulled
                        pulledGoldIndex = bestIndex; // Save gold index
                    }
                }
            }
            SDL_RenderClear(renderer); // Clear the renderer for a new frame
            if (!timeUp) { // If game is still running
                SDL_RenderCopy(renderer, bgTexture, NULL, NULL); // Render the background texture
                for (int i = 0; i < NUM_GOLDS; i++) { // Loop through gold objects
                    if (golds[i].active) { // If gold is active
                        if (golds[i].type == GOLD_MYSTERY) // If gold is a mystery type
                            SDL_RenderCopy(renderer, mysbagTexture, NULL, &golds[i].rect); // Render mystery bag texture
                        else // Otherwise
                            SDL_RenderCopy(renderer, goldTexture, NULL, &golds[i].rect); // Render gold texture
                    }
                }
                for (int i = 0; i < NUM_ROCKS; i++) { // Loop through rock objects
                    if (rocks[i].active) // If rock is active
                        SDL_RenderCopy(renderer, rockTexture, NULL, &rocks[i].rect); // Render rock texture
                }
                SDL_RenderCopy(renderer, charTexture, NULL, &charRect); // Render character texture
                float angleDeg = -(currentAngle * 180.0f / PI); // Convert current angle from radians to degrees (for rotation)
                if (hookState == dynamite_MOVING) { // If hook state is dynamite_MOVING
                    SDL_Rect dynamiteRect = { (int)hookX - hookW/2, (int)hookY - hookH/2, hookW, hookH }; // Define rectangle for dynamite at hook position
                    SDL_RenderCopyEx(renderer, dynamiteTexture, NULL, &dynamiteRect, angleDeg, &hookPivot, SDL_FLIP_NONE); // Render dynamite texture with rotation
                } else if (hookState == dynamite_EXPLOSION) { // If hook state is dynamite_EXPLOSION
                    SDL_Rect explosionRect; // Declare rectangle for explosion
                    explosionRect.x = (int)explosionX - hookW/2; // Set explosion X position based on hookX
                    explosionRect.y = (int)explosionY - hookH/2; // Set explosion Y position based on hookY
                    explosionRect.w = 100; // Set explosion width
                    explosionRect.h = 100; // Set explosion height
                    SDL_RenderCopy(renderer, implodeTexture, NULL, &explosionRect); // Render explosion texture
                } else { // Otherwise, in normal hook states
                    SDL_RenderCopyEx(renderer, hookTexture, NULL, &hookRect, angleDeg, &hookPivot, SDL_FLIP_NONE); // Render hook texture with rotation
                }
                int hookPivotScreenX = hookRect.x + hookPivot.x; // Calculate screen X coordinate of hook pivot point
                int hookPivotScreenY = hookRect.y + hookPivot.y; // Calculate screen Y coordinate of hook pivot point
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Set drawing color to red
                SDL_RenderDrawLine(renderer, (int)anchorX, (int)anchorY, hookPivotScreenX, hookPivotScreenY); // Draw a red line from character anchor to hook pivot
                char scoreText[32]; // Buffer for score text
                sprintf(scoreText, "Score: %d", score); // Format score text string
                SDL_Color whiteColor = {255, 255, 255, 255}; // Set white color for score text
                SDL_Surface* textSurface = TTF_RenderText_Blended(font, scoreText, whiteColor); // Render score text to a surface
                if (textSurface) { // If score text surface creation succeeded
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); // Create texture from score surface
                    SDL_FreeSurface(textSurface); // Free score surface
                    SDL_Rect textRect = {10, 10, 0, 0}; // Define rectangle for score display at (10,10)
                    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h); // Get score texture dimensions
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect); // Render score texture to screen
                    SDL_DestroyTexture(textTexture); // Destroy score texture to free memory
                }
                for (int i = 0; i < availabledynamites; i++) { // Loop for each available dynamite icon
                    SDL_Rect dRect = { charRect.x + charRect.w + i * 50, 50, 50, 50 }; // Define rectangle for dynamite icon
                    SDL_RenderCopy(renderer, dynamiteTexture, NULL, &dRect); // Render the dynamite icon
                }
                char timerText[32]; // Buffer for game timer text
                int minutes = ((int)gameTimer) / 60; // Calculate remaining minutes
                int seconds = ((int)gameTimer) % 60; // Calculate remaining seconds
                sprintf(timerText, "Time: %d:%02d", minutes, seconds); // Format timer text string
                SDL_Surface* timerSurface = TTF_RenderText_Blended(font, timerText, whiteColor); // Render timer text to a surface
                if (timerSurface) { // If timer surface created successfully
                    SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface); // Create texture from timer surface
                    SDL_FreeSurface(timerSurface); // Free timer surface
                    SDL_Rect timerRect = {10, 40, 0, 0}; // Define rectangle for timer display at (10,40)
                    SDL_QueryTexture(timerTexture, NULL, NULL, &timerRect.w, &timerRect.h); // Get timer texture dimensions
                    SDL_RenderCopy(renderer, timerTexture, NULL, &timerRect); // Render timer texture on screen
                    SDL_DestroyTexture(timerTexture); // Destroy timer texture to free memory
                }
            } else { // If game time is up, render failure screen during session loop
                SDL_Rect fullScreen = {0, 0, 1366, 768}; // Define full screen rectangle
                SDL_RenderCopy(renderer, failureTexture, NULL, &fullScreen); // Render failure texture full screen
            }
            SDL_RenderPresent(renderer); // Present current frame
            SDL_Delay(16); // Delay approximately 16 ms (≈60 frames per second)
        } // End of Game Session Loop
        SDL_Rect fullScreenRect = {0, 0, 1366, 768}; // Define full screen rectangle for post-game processing
        if (score >= 400) { // If player wins (score is at least 400)
            SDL_RenderClear(renderer); // Clear renderer
            SDL_RenderCopy(renderer, successTexture, NULL, &fullScreenRect); // Render success screen
            SDL_RenderPresent(renderer); // Present success screen
            SDL_Delay(4000); // Display success screen for 4 seconds
            // After showing success, return to main menu (do not load shop)
        } else { // If player loses (score < 400)
            SDL_RenderClear(renderer); // Clear renderer
            SDL_RenderCopy(renderer, failureTexture, NULL, &fullScreenRect); // Render failure screen
            SDL_RenderPresent(renderer); // Present failure screen
            SDL_Delay(4000); // Display failure screen for 4 seconds
        }
        // End of one game session; loop back to the main menu
    } // End of Main Session Loop
    SDL_DestroyTexture(hookTexture); // Destroy hook texture
    SDL_DestroyTexture(rockTexture); // Destroy rock texture
    SDL_DestroyTexture(goldTexture); // Destroy gold texture
    SDL_DestroyTexture(charTexture); // Destroy character texture
    SDL_DestroyTexture(bgTexture); // Destroy background texture
    SDL_DestroyTexture(dynamiteTexture); // Destroy dynamite texture
    SDL_DestroyTexture(implodeTexture); // Destroy explosion texture
    SDL_DestroyTexture(mysbagTexture); // Destroy mystery bag texture
    SDL_DestroyTexture(successTexture); // Destroy success texture
    SDL_DestroyTexture(failureTexture); // Destroy failure texture
    SDL_DestroyRenderer(renderer); // Destroy renderer
    SDL_DestroyWindow(window); // Destroy window
    TTF_CloseFont(font); // Close normal font
    TTF_CloseFont(font48); // Close larger font
    Mix_FreeMusic(targetMusic); // Free target music resource
    Mix_CloseAudio(); // Close audio system
    IMG_Quit(); // Quit SDL_image
    TTF_Quit(); // Quit SDL_ttf
    SDL_Quit(); // Quit SDL subsystems
    return 0; // Exit program successfully
}
void showTargetScreen(SDL_Renderer* renderer, TTF_Font* font48, SDL_Texture* targetTexture, Mix_Music* targetMusic, int neededPoints) { // Function to display the target screen
    if (targetMusic) // If target music is available
        Mix_PlayMusic(targetMusic, 1); // Play target music once
    char targetText[64]; // Buffer for the target points text
    sprintf(targetText, "%d points", neededPoints); // Format the target points text
    SDL_Color white = {255,255,255,255}; // White color for text
    SDL_Surface* textSurface = TTF_RenderText_Blended(font48, targetText, white); // Render the target text onto a surface using 48pt font
    if (!textSurface) { // Check if text rendering failed
        printf("Error rendering target text: %s\n", TTF_GetError()); // Print error message
        return; // Exit function if error occurs
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); // Create texture from the text surface
    SDL_FreeSurface(textSurface); // Free the text surface after texture creation
    int textW, textH; // Variables to hold the dimensions of the text texture
    SDL_QueryTexture(textTexture, NULL, NULL, &textW, &textH); // Get the dimensions of the text texture
    Uint32 startTime = SDL_GetTicks(); // Record current time for display duration
    while (SDL_GetTicks() - startTime < 4000) { // Loop for 4 seconds
        SDL_RenderClear(renderer); // Clear renderer for new frame
        SDL_RenderCopy(renderer, targetTexture, NULL, NULL); // Render the target background texture
        SDL_Rect textRect = { (1366 - textW)/2, (768 - textH)/2, textW, textH }; // Define rectangle to center the text on screen
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect); // Render the text texture onto the screen
        SDL_RenderPresent(renderer); // Present the rendered frame
        SDL_Delay(16); // Delay approximately 16 ms (about 60fps)
    }
    SDL_DestroyTexture(textTexture); // Clean up the text texture
}
void showControls(SDL_Renderer* renderer, TTF_Font* font) { // Function to display control instructions overlay
    bool done = false; // Flag to indicate when to exit the controls screen
    SDL_Event e; // Declare event variable for controls screen
    while (!done) { // Loop until user decides to exit controls
        while (SDL_PollEvent(&e)) { // Poll for events
            if (e.type == SDL_QUIT) // If quit event is received
                done = true; // Set done flag to true
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) // If ESC key is pressed
                done = true; // Set done flag to true
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Set background color to semi-transparent black
        SDL_RenderClear(renderer); // Clear renderer with chosen background color
        SDL_Color white = {255,255,255,255}; // Define white color for text
        const char* line1 = "Controls:"; // First line of controls text
        const char* line2 = "Up button = Dynamite"; // Instruction for dynamite activation
        const char* line3 = "Down button = Lower Mining Crank"; // Instruction for lowering the hook
        const char* line4 = "(Press ESC to go back)"; // Instruction on how to exit the controls screen
        auto renderLine = [renderer, font, white](const char* text, int yOffset) { // Lambda function to render a single line of text at a given y offset
            SDL_Surface* surf = TTF_RenderText_Blended(font, text, white); // Render text to a surface
            if (!surf) return; // If surface creation fails, exit lambda
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf); // Create texture from the surface
            SDL_FreeSurface(surf); // Free the surface
            SDL_Rect rect = {0,0,0,0}; // Declare rectangle to hold texture dimensions and position
            SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h); // Get texture dimensions
            rect.x = (1366 - rect.w)/2; // Center text horizontally on screen
            rect.y = yOffset; // Set vertical position as specified
            SDL_RenderCopy(renderer, tex, NULL, &rect); // Render the texture onto the renderer
            SDL_DestroyTexture(tex); // Destroy the texture
        };
        renderLine(line1, 150); // Render first line at y=150
        renderLine(line2, 200); // Render second line at y=200
        renderLine(line3, 240); // Render third line at y=240
        renderLine(line4, 300); // Render fourth line at y=300
        SDL_RenderPresent(renderer); // Present the rendered controls overlay
        SDL_Delay(16); // Delay to maintain approximately 60fps
    }
}
void showHighScores(SDL_Renderer* renderer, TTF_Font* font) { // Function to display high scores screen
    bool done = false; // Flag to indicate when to exit high scores screen
    SDL_Event e; // Declare event variable for high scores screen
    while (!done) { // Loop until the user decides to exit
        while (SDL_PollEvent(&e)) { // Poll for events
            if (e.type == SDL_QUIT) // If quit event is received
                done = true; // Set done flag
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) // If ESC key is pressed
                done = true; // Set done flag
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Set semi-transparent black background color
        SDL_RenderClear(renderer); // Clear the renderer with the background color
        SDL_Color white = {255,255,255,255}; // Define white color for text
        const char* msg = "High Scores - Not implemented yet.\n(Press ESC to go back)"; // Set the high scores message
        SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, msg, white, 600); // Render the text wrapped to 600px width
        if (surf) { // If the text surface is successfully created
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf); // Create texture from the surface
            SDL_FreeSurface(surf); // Free the surface
            SDL_Rect rect = {0,0,0,0}; // Declare rectangle for the texture
            SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h); // Get texture dimensions
            rect.x = (1366 - rect.w)/2; // Center the rectangle horizontally
            rect.y = (768 - rect.h)/2; // Center the rectangle vertically
            SDL_RenderCopy(renderer, tex, NULL, &rect); // Render the high scores texture
            SDL_DestroyTexture(tex); // Destroy the high scores texture
        }
        SDL_RenderPresent(renderer); // Present the rendered high scores frame
        SDL_Delay(16); // Delay to maintain approximately 60fps
    }
}
int runMenu(SDL_Renderer* renderer, TTF_Font* font) { // Function to run the main menu screen
    SDL_Surface* menuBGSurface = IMG_Load("daovang.png"); // Load the menu background image
    if (!menuBGSurface) { // If the menu background fails to load
        printf("Error loading daovang.png: %s\n", IMG_GetError()); // Print error message
        return 1; // Return error code
    }
    SDL_Texture* menuBGTexture = SDL_CreateTextureFromSurface(renderer, menuBGSurface); // Create texture from the menu background image
    SDL_FreeSurface(menuBGSurface); // Free the menu background surface
    SDL_Rect goldRect = {420,150,200,200}; // Define clickable area for "Begin"
    SDL_Rect controlRect = {300,500,200,50}; // Define clickable area for "Control" button
    SDL_Rect scoresRect = {500,500,200,50}; // Define clickable area for "High Scores" button
    SDL_Event e; // Declare event variable for the menu
    bool running = true; // Boolean flag for the menu loop
    while (running) { // Begin main menu loop
        while (SDL_PollEvent(&e)) { // Poll for menu events
            if (e.type == SDL_QUIT) { // If a quit event is received while in menu
                SDL_DestroyTexture(menuBGTexture); // Destroy the menu background texture
                return 1; // Return quit signal
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) { // If left mouse button is pressed
                int mx = e.button.x; // Get mouse x coordinate
                int my = e.button.y; // Get mouse y coordinate
                if (mx >= goldRect.x && mx <= goldRect.x + goldRect.w && my >= goldRect.y && my <= goldRect.y + goldRect.h) { // If click is inside "Begin" area
                    SDL_DestroyTexture(menuBGTexture); // Destroy menu background texture
                    return 0; // Return 0 to signal starting a new game session
                }
                if (mx >= controlRect.x && mx <= controlRect.x + controlRect.w && my >= controlRect.y && my <= controlRect.y + controlRect.h) { // If click is within "Control" button area
                    showControls(renderer, font); // Display the controls overlay
                }
                if (mx >= scoresRect.x && mx <= scoresRect.x + scoresRect.w && my >= scoresRect.y && my <= scoresRect.y + scoresRect.h) { // If click is within "High Scores" area
                    showHighScores(renderer, font); // Display the high scores screen
                }
            }
        }
        SDL_RenderClear(renderer); // Clear the renderer for the menu frame
        SDL_RenderCopy(renderer, menuBGTexture, NULL, NULL); // Render the menu background texture
        SDL_SetRenderDrawColor(renderer, 0,255,0,100); // Set draw color to semi-transparent green for the control button background
        SDL_RenderFillRect(renderer, &controlRect); // Fill the control button rectangle with green
        SDL_SetRenderDrawColor(renderer, 0,0,255,100); // Set draw color to semi-transparent blue for the high scores button background
        SDL_RenderFillRect(renderer, &scoresRect); // Fill the high scores button rectangle with blue
        SDL_Color white = {255,255,255,255}; // Define white color for text
        SDL_Surface* bgnSurf = TTF_RenderText_Blended(font, "Begin", white); // Render "Begin" text onto a surface
        if (bgnSurf) { // If "Begin" text surface creation succeeded
            SDL_Texture* bgnText = SDL_CreateTextureFromSurface(renderer, bgnSurf); // Create texture from "Begin" text surface
            SDL_FreeSurface(bgnSurf); // Free the "Begin" text surface
            SDL_Rect bgnTextRect = {goldRect.x, goldRect.y, 0, 0}; // Define rectangle for "Begin" text
            SDL_QueryTexture(bgnText, NULL, NULL, &bgnTextRect.w, &bgnTextRect.h); // Get texture dimensions
            bgnTextRect.x += (goldRect.w - bgnTextRect.w) / 2; // Center "Begin" text horizontally within goldRect
            bgnTextRect.y += (goldRect.h - bgnTextRect.h) / 2; // Center "Begin" text vertically within goldRect
            SDL_RenderCopy(renderer, bgnText, NULL, &bgnTextRect); // Render "Begin" texture
            SDL_DestroyTexture(bgnText); // Destroy "Begin" texture after rendering
        }
        SDL_Surface* ctrlSurf = TTF_RenderText_Blended(font, "Control", white); // Render "Control" text onto a surface
        if (ctrlSurf) { // If creation succeeds
            SDL_Texture* ctrlText = SDL_CreateTextureFromSurface(renderer, ctrlSurf); // Create texture from "Control" surface
            SDL_FreeSurface(ctrlSurf); // Free the "Control" surface
            SDL_Rect ctrlTextRect = {controlRect.x, controlRect.y, 0, 0}; // Define rectangle for "Control" text
            SDL_QueryTexture(ctrlText, NULL, NULL, &ctrlTextRect.w, &ctrlTextRect.h); // Get texture dimensions
            ctrlTextRect.x += (controlRect.w - ctrlTextRect.w) / 2; // Center horizontally within controlRect
            ctrlTextRect.y += (controlRect.h - ctrlTextRect.h) / 2; // Center vertically within controlRect
            SDL_RenderCopy(renderer, ctrlText, NULL, &ctrlTextRect); // Render "Control" texture
            SDL_DestroyTexture(ctrlText); // Destroy "Control" texture after rendering
        }
        SDL_Surface* scoresSurf = TTF_RenderText_Blended(font, "High Scores", white); // Render "High Scores" text onto a surface
        if (scoresSurf) { // If creation succeeds
            SDL_Texture* scoresText = SDL_CreateTextureFromSurface(renderer, scoresSurf); // Create texture for "High Scores"
            SDL_FreeSurface(scoresSurf); // Free the "High Scores" surface
            SDL_Rect scoresTextRect = {scoresRect.x, scoresRect.y, 0, 0}; // Define rectangle for text
            SDL_QueryTexture(scoresText, NULL, NULL, &scoresTextRect.w, &scoresTextRect.h); // Get dimensions of the texture
            scoresTextRect.x += (scoresRect.w - scoresTextRect.w) / 2; // Center horizontally within scoresRect
            scoresTextRect.y += (scoresRect.h - scoresTextRect.h) / 2; // Center vertically within scoresRect
            SDL_RenderCopy(renderer, scoresText, NULL, &scoresTextRect); // Render the "High Scores" texture
            SDL_DestroyTexture(scoresText); // Destroy the texture to free memory
        }
        SDL_RenderPresent(renderer); // Present the rendered menu frame
        SDL_Delay(16); // Delay approximately 16 ms to cap frame rate at ~60fps
    }
    SDL_DestroyTexture(menuBGTexture); // Destroy the menu background texture
    return 1; // Return 1 if menu loop exits unexpectedly
}
