//
// Created by Oscar Holroyd on 18/01/2025.
//

#include "game_tileset.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "src/wordlists/en-gb.h"


// Check that the alphabet is contiguous
static_assert('z' - 'a' == 25, "alphabet chars must be contiguous");


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


char score_letter_tileset(const char letter) {
  if (letter == ' ') {
    return 0;
  }

  return SCORES[letter - 'a'];
}


char score_word_tileset(const struct Game *game, const char *word) {
  char used[SIZE];
  memset(used, 0, SIZE);

  char score = 0;
  for (int i = 0; word[i] != '\0'; i++) {
    // check that the letter is available
    int was_found = 0;
    for (int j = 0; j < SIZE; j++) {
      if (word[i] == game->letters[j] && !used[j]) {
        used[j] = 1;
        score += score_letter_tileset(word[i]);
        was_found = 1;
        break;
      }
    }

    // if the letter was not available, try to use a blank (which carries no point value)
    if (!was_found) {
      for (int j = 0; j < SIZE; j++) {
        if (game->letters[j] == ' ' && !used[j]) {
          used[j] = 1;
          was_found = 1;
          break;
        }
      }
    }

    // if the letter was not found then the word is not valid
    if (!was_found) {
      return 0;
    }
  }
  return score;
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
    const char j = i + rand() / (RAND_MAX / (SIZE - i) + 1);
    const char tmp = game->letters[j];
    game->letters[j] = game->letters[i];
    game->letters[i] = tmp;
  }
}


int submit_word_tileset(struct Game *game, const char *word) {
  // try and find the word in the dictionary
  const int num_words = NUM_EN_GB;
  for (int i = 0; i < num_words; ++i) {
    if (!strcmp(word, EN_GB[i])) {
      // word found
      char score = score_word_tileset(game, word);

      // check if the word is better than the current best
      if (score > game->score) {
        game->score = score;
        strcpy(game->word, word);
      }
      return score;
    }
  }

  return 0;
}
