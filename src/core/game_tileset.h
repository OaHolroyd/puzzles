//
// Created by Oscar Holroyd on 18/01/2025.
//

#ifndef GAME_TILESET_H
#define GAME_TILESET_H

#define SIZE (7) // the number of letters available
#define STORE (10) // the number of top words to store
#define BLANK (' ') // the blank tile


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
 * Since the depth first search to find the best words is expensive, this can be disabled.
 *
 * @param game The game state.
 * @param get_top_words Whether to get the top words (0 to skip, nonzero to perform).
 */
void reset_tileset(struct Game *game, int get_top_words);


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
int submit_word_tileset(struct Game *game, const char *word);


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
int score_word_tileset(const struct Game *game, const char *word);


/**
 * Perform a depth first search to find the best words.
 *
 * @param game The game state.
 */
void top_words_tileset(struct Game *game);


/**
 * Find the blanks in a word.
 *
 * @param game The game state.
 * @param word The word to find the blanks in.
 * @param blanks When the function returns, this will be 1 for a blank, 0 for a normal letter.
 */
void find_blanks_tileset(struct Game *game, const char *word, char *blanks);


#endif //GAME_TILESET_H
