#ifndef HIGH_SCORES_H
#define HIGH_SCORES_H

#include <SDL.h>
#include <SDL_ttf.h>

// Updates the high-scores file with the new score.
// The file "highscores.txt" stores up to 5 scores (sorted descending).
// If fewer than 5 exist, the vacant positions remain empty.
void updateHighScores(int newScore);

// Reads from the high-scores file and renders the 5 high-score lines.
void showHighScores(SDL_Renderer* renderer, TTF_Font* font);

#endif // HIGH_SCORES_H
