//
// Created by Oscar Holroyd on 18/01/2025.
//

#ifndef GAME_TILESET_H
#define GAME_TILESET_H

#define SIZE (7) // the number of letters available
#define STORE (10) // the number of top words to store

/**
 * Game state.
 */
struct Game {
  char letters[SIZE + 1]; // letters available for use (always null-terminated)

  char word[SIZE + 1]; // the best word found so far (always null-terminated)
  int score; // current score

  char top_words[STORE][SIZE + 1]; // best available words (always null-terminated)
  int top_scores[STORE]; // scores of the best available words
};


/**
 * Reset the game state, choosing a random set of letters.
 *
 * @param game The game state.
 */
void reset_tileset(struct Game *game);


/**
 * Shuffle the letters in the game state.
 *
 * @param game The game state.
 */
void shuffle_tileset(struct Game *game);


#endif //GAME_TILESET_H
