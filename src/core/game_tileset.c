//
// Created by Oscar Holroyd on 18/01/2025.
//

#include "game_tileset.h"

#include <stdlib.h>
#include <assert.h>


// Check that the alphabet is contiguous
static_assert('z' - 'a' == 25);


// Available letters, with their frequencies
static const char LETTERS[100] = {
  'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
  'b', 'b',
  'c', 'c',
  'd', 'd', 'd', 'd',
  'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e',
  'f', 'f',
  'g', 'g', 'g',
  'h', 'h',
  'i', 'i', 'i', 'i', 'i', 'i', 'i', 'i', 'i',
  'j',
  'k',
  'l', 'l', 'l', 'l',
  'm', 'm',
  'n', 'n', 'n', 'n', 'n', 'n',
  'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o',
  'p', 'p',
  'q',
  'r', 'r', 'r', 'r', 'r', 'r',
  's', 's', 's', 's',
  't', 't', 't', 't', 't', 't',
  'u', 'u', 'u', 'u',
  'v', 'v',
  'w', 'w',
  'x',
  'y', 'y',
  'z',
  ' ', ' ',
};


// Scores for each letter
static const char SCORES[26] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10,};
//                              a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p,  q, r, s, t, u, v, w, x, y,  z


/**
 * Get the score of a letter.
 *
 * Note that this assumes lowercase letters, and contiguous values of the chars 'a' to 'z'.
 *
 * @param letter The letter to get the score of.
 * @return The score of the letter.
 */
static char score_letter(const char letter) {
  if (letter == ' ') {
    return 0;
  }

  return SCORES[letter - 'a'];
}


void reset_tileset(struct Game *game) {
  game->score = 0;

  /* pick random indices into the LETTERS array, not allowing replacement */
  for (char i = 0; i < SIZE; i++) {
    int redraw;
    do {
      // get a random index
      game->letters[i] = rand() % 100;

      // check if the letter is already in the shuffle
      redraw = 0;
      for (char j = 0; j < i; j++) {
        if (game->letters[i] == game->letters[j]) {
          redraw = 1;
          break;
        }
      }
    } while (redraw);
  }

  /* convert indices into letters */
  for (char i = 0; i < SIZE; i++) {
    game->letters[i] = LETTERS[game->letters[i]];
  }
  game->letters[SIZE] = '\0'; // ensure the string is null-terminated

  // TODO: find top words/scores (this requires a prefix-tree of the entire dictionary)
}


void shuffle_tileset(struct Game *game) {
  // Perform a Fisher-Yates shuffle
  for (char i = 0; i < SIZE - 1; i++) {
    char j = i + rand() / (RAND_MAX / (SIZE - i) + 1);
    char tmp = game->letters[j];
    game->letters[j] = game->letters[i];
    game->letters[i] = tmp;
  }
}
