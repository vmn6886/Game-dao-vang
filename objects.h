#ifndef OBJECTS_H
#define OBJECTS_H

#include <SDL.h>
#include <stdbool.h>

// Define the number of objects
#define NUM_GOLDS 11
#define NUM_ROCKS 4

// Define gold types.
typedef enum { GOLD_SMALL, GOLD_MEDIUM, GOLD_BIG, GOLD_MYSTERY } GoldType;

// Define rock types.
typedef enum { ROCK_SMALL, ROCK_BIG } RockType;

// Structure for a gold object.
typedef struct {
    SDL_Rect rect;
    GoldType type;
    bool active;
} GoldObject;

// Structure for a rock object.
typedef struct {
    SDL_Rect rect;
    RockType type;
    bool active;
} RockObject;

#endif // OBJECTS_H
