//
// Created by Oscar Holroyd on 18/01/2025.
//

#include "game_tileset.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "src/wordlists/en-gb.h"
#include "src/core/trie.h"


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
  BLANK, BLANK,
};


// Scores for each letter
static const char SCORES[26] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10,};
//                              a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p,  q, r, s, t, u, v, w, x, y,  z


/**
 * Check if a seed is valid.
 *
 * The seed must be a string representing a 7 two-digit hex numbers between 0 and 99 (inclusive).
 *
 * @param seed The seed to check.
 * @return 0 on success, non-zero on failure.
 */
int parse_seed(char *letters, const char *seed) {
  // must have length 14, or 16 with leading "0x"
  long hex_seed;
  char *endptr;
  if (strnlen(seed, 17) == 16 && seed[0] == '0' && seed[1] == 'x') {
    hex_seed = strtol(seed, &endptr, 0);
  } else if (strnlen(seed, 17) != 14) {
    return 1; // incorrect length
  } else {
    hex_seed = strtol(seed, &endptr, 16);
  }

  if (endptr[0] != '\0') {
    return 2; // invalid hex characters
  }

  // extract the 7 hex numbers
  for (int i = 0; i < SIZE; i++) {
    const int value = (hex_seed >> (8 * (SIZE - i - 1))) & 0xFF;
    if (value > 99) {
      return 3; // out of bounds
    }

    for (int j = 0; j < i; j++) {
      if (letters[j] == value) {
        return 4; // repeated index
      }
    }

    letters[i] = value;
  }

  return 0;
}


/**
 * Depth-first search of the trie, finding the best words.
 *
 * @param root the root of the trie
 * @param game the game state
 * @param prefix the current prefix
 * @param used the letters that have been used
 * @param len the length of the prefix
 */
static void depth_first_search(
  Trie *root, struct Game *game, const char prefix[SIZE + 1], const char used[SIZE + 1], const int len
) {
  for (int i = 0; i < SIZE; i++) {
    if (used[i]) {
      continue;
    }

    /* set up the possible letters the letters[i] could be */
    char alphabet[26] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    };
    if (game->letters[i] != BLANK) {
      alphabet[0] = game->letters[i];
      alphabet[1] = '\0';
    }

    for (int k = 0; k < 26; k++) {
      if (alphabet[k] == '\0') {
        break;
      }

      /* duplicate the input arguments */
      char newPrefix[SIZE + 1] = {0};
      char newPrefixBlank[SIZE + 1] = {0}; // newPrefix using the blank tile (if it exists)
      char newUsed[SIZE + 1] = {0};
      memcpy(newPrefix, prefix, len);
      memcpy(newPrefixBlank, prefix, len);
      memcpy(newUsed, used, SIZE);

      /* add the letter to the end of the prefix */
      newUsed[i] = 1;
      newPrefix[len] = alphabet[k]; // newPrefix can never have a blank in it since we use it to check the dictionary
      newPrefix[len + 1] = '\0';
      newPrefixBlank[len] = game->letters[i]; // newPrefixBlank
      newPrefixBlank[len + 1] = '\0';

      /* previous letters might also have been a blank, so replace them */
      for (int j = 0; j < len; j++) {
        int is_blank = 1;
        for (int p = 0; p < SIZE; p++) {
          if (used[p] && game->letters[p] == newPrefix[j]) {
            is_blank = 0;
            break;
          }
        }

        if (is_blank) {
          newPrefixBlank[j] = BLANK;
        }
      }

      /* check if this prefix is a real word */
      if (trie_contains(root, newPrefix, 0)) {
        /* don't bother adding words that are already in the lists */
        int is_new = 1;
        for (int j = 0; j < STORE; j++) {
          if (!strncmp(game->top_words[j], newPrefix, SIZE)) {
            is_new = 0;
            break;
          }
        }

        if (is_new) {
          /* find the worst of the current best words */
          int worst_score = 10000000;
          int worst_index = 0;
          for (int j = 0; j < STORE; j++) {
            if (
              game->top_scores[j] < worst_score ||
              (game->top_scores[j] == worst_score && strncmp(game->top_words[j], game->top_words[worst_index], SIZE) >
               0)
            ) {
              worst_score = game->top_scores[j];
              worst_index = j;
            }
          }

          /* replace it with this one if it is better (or the same and lower alphabetically) */
          const char score = score_word_tileset(game, newPrefixBlank);
          if (
            score > worst_score ||
            (score == worst_score && strncmp(newPrefix, game->top_words[worst_index], SIZE) < 0)
          ) {
            game->top_scores[worst_index] = score;
            memcpy(game->top_words[worst_index], newPrefix, SIZE);
          }
        }
      } else if (!trie_contains(root, newPrefix, 1)) {
        continue;
      }

      /* try to continue with more letters since this is a valid prefix */
      depth_first_search(root, game, newPrefix, newUsed, len + 1);
    }
  }
}


char score_letter_tileset(const char letter) {
  if (letter == BLANK) {
    return 0;
  }

  return SCORES[letter - 'a'];
}


int score_word_tileset(const struct Game *game, const char *word) {
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
        if (game->letters[j] == BLANK && !used[j]) {
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


int reset_tileset(struct Game *game, const char *seed, const int get_top_words) {
  game->score = 0;
  memset(game->top_scores, 0, STORE * sizeof(int));
  memset(game->has_found, 0, STORE * sizeof(int));
  for (int i = 0; i < STORE; i++) {
    memset(game->top_words[i], 0, SIZE + 1);
  }

  if (seed) {
    /* set indices into the letters array from the seed */
    if (parse_seed(game->letters, seed)) {
      return 1;
    }
  } else {
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
  }

  /* convert indices into letters */
  for (char i = 0; i < SIZE; i++) {
    game->letters[i] = LETTERS[game->letters[i]];
  }
  game->letters[SIZE] = '\0'; // ensure the string is null-terminated

  if (get_top_words) {
    top_words_tileset(game);
  }

  return 0;
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
  int score = 0;

  /* try and find the word in the dictionary */
  const int num_words = NUM_EN_GB;
  for (int i = 0; i < num_words; ++i) {
    if (!strcmp(word, EN_GB[i])) {
      // word found
      score = score_word_tileset(game, word);

      // check if the word is better than the current best
      if (score > game->score) {
        game->score = score;
        strcpy(game->word, word);
      }
      break;
    }
  }

  /* if the word was found, see if it matches any of the top words */
  if (score > 0) {
    for (int i = 0; i < STORE; i++) {
      if (!strncmp(game->top_words[i], word, SIZE)) {
        game->has_found[i] = 1;
        break;
      }
    }
  }

  return score;
}


void top_words_tileset(struct Game *game) {
  /* construct a prefix tree containing all of the words in the dictionary */
  Trie root = trie_root();
  for (int i = 0; i < NUM_EN_GB; i++) {
    trie_insert(&root, EN_GB[i]);
  }

  /* do a depth first search of the trie to find the best words */
  const char prefix[SIZE + 1] = {0};
  const char used[SIZE + 1] = {0};
  depth_first_search(&root, game, prefix, used, 0);

  trie_free(&root);

  /* sort the words from best to worst */
  // use shell sort
  int k = 0;
  while (++k) {
    const int gap = 2 * (STORE >> (k + 1)) + 1;

    for (int i = gap; i < STORE; i++) {
      for (int j = i - gap; j >= 0; j -= gap) {
        // determine if the current word is better than the next (by score then alphabetically)
        int is_better = game->top_scores[j] < game->top_scores[j + gap];
        if (game->top_scores[j] == game->top_scores[j + gap]) {
          is_better = strcmp(game->top_words[j], game->top_words[j + gap]) > 0;
        }

        if (is_better) {
          // swap the scores
          const int tmp_score = game->top_scores[j];
          game->top_scores[j] = game->top_scores[j + gap];
          game->top_scores[j + gap] = tmp_score;

          // swap the words
          char tmp_word[SIZE + 1];
          strcpy(tmp_word, game->top_words[j]);
          strcpy(game->top_words[j], game->top_words[j + gap]);
          strcpy(game->top_words[j + gap], tmp_word);
        }
      }
    }

    if (gap == 1) {
      // when gap == 1 then the sort is complete
      break;
    }
  }
}


void find_blanks_tileset(const struct Game *game, const char *word, char *blanks) {
  char used[SIZE] = {0};
  memset(blanks, 0, SIZE);

  for (int i = 0; i < SIZE; i++) {
    if (!word[i]) {
      // stop at the end of the word
      return;
    }

    /* try and find the letter in the tileset */
    int found = 0;
    for (int j = 0; j < SIZE; j++) {
      if (game->letters[j] == word[i] && !used[j]) {
        used[j] = 1;
        found = 1;
        break;
      }
    }
    if (found) {
      continue;
    }

    /* if the letter was not found, try to use a blank */
    for (int j = 0; j < SIZE; j++) {
      if (game->letters[j] == BLANK && !used[j]) {
        used[j] = 1;
        blanks[i] = 1;
        break;
      }
    }
  }
}


void reveal_tileset(struct Game *game) {
  for (int i = 0; i < STORE; i++) {
    game->has_found[i] = 1;
  }
}
