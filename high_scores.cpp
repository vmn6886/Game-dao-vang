#include "high_scores.h"
#include <stdio.h>
#include <stdlib.h>

void updateHighScores(int newScore) {
    int scores[5];
    int count = 0;

    // Open the file for reading the current high scores.
    FILE* file = fopen("highscores.txt", "r");
    if (file != NULL) {
        char line[128];
        // Read up to 5 lines (scores) from the file.
        while (fgets(line, sizeof(line), file) != NULL && count < 5) {
            int score;
            // Expect the format "Rank: score". If not found, assume score is 0.
            if (sscanf(line, "%*d: %d", &score) == 1) {
                scores[count++] = score;
            } else {
                scores[count++] = 0;
            }
        }
        fclose(file);
    }
    // Check if there is room (i.e. fewer than 5 scores) or if the new score is higher than at least one.
    if (count < 5) {
        // There are less than 5 scores; add the new score.
        scores[count++] = newScore;
    } else {
        // Find the minimum (lowest) score among the five.
        int minIndex = 0;
        for (int i = 1; i < count; i++) {
            if (scores[i] < scores[minIndex])
                minIndex = i;
        }
        // If the new score is higher than the smallest score, replace it.
        if (newScore > scores[minIndex])
            scores[minIndex] = newScore;
    }
    // Sort the scores in descending order using a simple bubble sort.
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (scores[j] > scores[i]) {
                int temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }
    }
    // Write back exactly five lines to the file.
    file = fopen("highscores.txt", "w");
    if (file != NULL) {
        for (int i = 0; i < 5; i++) {
            if (i < count && scores[i] > 0)
                fprintf(file, "%d: %d\n", i + 1, scores[i]);
            else
                fprintf(file, "%d:\n", i + 1);
        }
        fclose(file);
    }
}

void showHighScores(SDL_Renderer* renderer, TTF_Font* font) {
    // First, updateHighScores with a dummy score (0) so that the file exists
    updateHighScores(0);
    char lines[5][128] = { {0} };
    FILE* file = fopen("highscores.txt", "r");
    if (file != NULL) {
        for (int i = 0; i < 5; i++) {
            if (fgets(lines[i], sizeof(lines[i]), file) == NULL)
                lines[i][0] = '\0';
        }
        fclose(file);
    }
    bool done = false;
    SDL_Event e;
    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                done = true;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                done = true;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black background
        SDL_RenderClear(renderer);
        SDL_Color white = {255, 255, 255, 255};
        // Render each high score line (placed with some left margin and vertical spacing)
        for (int i = 0; i < 5; i++) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, lines[i], white);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FreeSurface(surf);
                SDL_Rect rect;
                rect.x = 100;
                rect.y = 150 + i * 50;
                SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);
                SDL_RenderCopy(renderer, tex, NULL, &rect);
                SDL_DestroyTexture(tex);
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
