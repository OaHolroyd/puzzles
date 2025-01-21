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


/**
 * Submit a word to the game state.
 *
 * @param game The game state.
 * @param word The word to submit.
 * @return the score value of the word if the word is correct, 0 if the word is incorrect.
 */
int submit_word_tileset(const struct Game *game, const char *word);


/**
 * Get the score of a letter.
 *
 * Note that this assumes lowercase letters, and contiguous values of the chars 'a' to 'z'.
 *
 * @param letter The letter to get the score of.
 * @return The score of the letter.
 */
char score_letter_tileset(char letter);


/**
 * Get the score of a null-terminated string.
 *
 * Note that this assumes lowercase letters, and contiguous values of the chars 'a' to 'z'.
 *
 * @param game The game state.
 * @param word The word to get the score of.
 * @return The score of the word, or 0 if the word is invalid.
 */
char score_word_tileset(const struct Game *game, const char *word);


#endif //GAME_TILESET_H
