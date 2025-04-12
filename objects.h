#ifndef OBJECTS_H
#define OBJECTS_H

#include <SDL.h>
#include <stdbool.h>

// Gold types
typedef enum {
    GOLD_SMALL,
    GOLD_MEDIUM,
    GOLD_BIG,
    GOLD_MYSTERY
} GoldType;

// Rock types
typedef enum {
    ROCK_SMALL,
    ROCK_BIG
} RockType;

// Structure for a Gold Object
typedef struct {
    SDL_Rect rect;
    GoldType type;
    bool active;
} GoldObject;

// Structure for a Rock Object
typedef struct {
    SDL_Rect rect;
    RockType type;
    bool active;
} RockObject;

#endif // OBJECTS_H
