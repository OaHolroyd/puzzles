#include "testing.h"

#include <string.h>

#include "src/core/game_tileset.h"


int main(void) {
  START_TEST("tileset");

  /* check that a shuffle always keeps the letters the same */
  SUBTEST("shuffle") {
    struct Game game;
    reset_tileset(&game, 0);

    char letters[SIZE + 1];
    strcpy(letters, game.letters); // store the original letters

    for (int i = 0; i < 50; i++) {
      strcpy(game.letters, letters); // restore the original letters
      shuffle_tileset(&game);

      /* compare */
      for (int j = 0; j < SIZE; j++) {
        char letter = letters[j];

        int found = 0;
        for (int k = 0; k < SIZE; k++) {
          if (game.letters[k] == letter) {
            game.letters[k] = 0;
            found = 1;
            break;
          }
        }
        REQUIRE(found == 1);
      }
    }
  }

  /* check that reseting never draws the same tile twice */
  SUBTEST("reset") {
    struct Game game;
    reset_tileset(&game, 0);

    for (int i = 0; i < 500000; i++) {
      reset_tileset(&game, 0);

      /* check that there's only one j, k, q, x, or z */
      const char jkqxz[5] = {'j', 'k', 'q', 'x', 'z'};
      for (int k = 0; k < 5; k++) {
        // check that there's only one
        char ch = jkqxz[k];
        char count = 0;
        for (int j = 0; j < SIZE; j++) {
          if (game.letters[j] == ch) {
            count++;
          }
        }
        REQUIRE(count <= 1);
      }
    }
  }

  /* check that words are scored correctly */
  SUBTEST("score word") {
    struct Game game;
    reset_tileset(&game, 0);

    /* no blanks */
    memcpy(game.letters, "abcdefg", 7);
    REQUIRE(score_word_tileset(&game, "bed") == 6); // valid real word
    REQUIRE(score_word_tileset(&game, "beg") == 6); // valid real word
    REQUIRE(score_word_tileset(&game, "abd") == 6); // words don't need to be correct, just valid letter combinations
    REQUIRE(score_word_tileset(&game, "bez") == 0); // invalid letter combination
    REQUIRE(score_word_tileset(&game, "bqz") == 0); // invalid letter combination

    /* one blank */
    memcpy(game.letters, "abcdef ", 7);
    REQUIRE(score_word_tileset(&game, "bed") == 6); // valid real word
    REQUIRE(score_word_tileset(&game, "beg") == 4); // valid real word (now uses blank)
    REQUIRE(score_word_tileset(&game, "abd") == 6); // words don't need to be correct, just valid letter combinations
    REQUIRE(score_word_tileset(&game, "bez") == 4); // valid letter combination using the blank
    REQUIRE(score_word_tileset(&game, "bqz") == 0); // invalid letter combination

    /* two blanks */
    memcpy(game.letters, "abcde  ", 7);
    REQUIRE(score_word_tileset(&game, "bed") == 6); // valid real word
    REQUIRE(score_word_tileset(&game, "beg") == 4); // valid real word (now uses blank)
    REQUIRE(score_word_tileset(&game, "abd") == 6); // words don't need to be correct, just valid letter combinations
    REQUIRE(score_word_tileset(&game, "bez") == 4); // valid letter combination using the blank
    REQUIRE(score_word_tileset(&game, "bqz") == 3); // valid letter combination using both blanks
  }

  /* check that words are submitted correctly */
  SUBTEST("submit word") {
    struct Game game;
    reset_tileset(&game, 0);

    /* no blanks */
    memcpy(game.letters, "abcdefg", 7);
    REQUIRE(submit_word_tileset(&game, "bed") == 6); // valid real word
    REQUIRE(submit_word_tileset(&game, "beg") == 6); // valid real word
    REQUIRE(submit_word_tileset(&game, "abd") == 0); // valid letters, not a word
    REQUIRE(submit_word_tileset(&game, "gaze") == 0); // real word, invalid letters
    REQUIRE(submit_word_tileset(&game, "quad") == 0); // real word, invalid letters

    /* one blank */
    memcpy(game.letters, "abcde g", 7);
    REQUIRE(submit_word_tileset(&game, "bed") == 6); // valid real word
    REQUIRE(submit_word_tileset(&game, "beg") == 6); // valid real word
    REQUIRE(submit_word_tileset(&game, "abd") == 0); // valid letters, not a word
    REQUIRE(submit_word_tileset(&game, "gaze") == 4); // real word, valid letters (using a blank)
    REQUIRE(submit_word_tileset(&game, "quad") == 0); // real word, invalid letters

    /* two blanks */
    memcpy(game.letters, "abcde  ", 7);
    REQUIRE(submit_word_tileset(&game, "bed") == 6); // valid real word
    REQUIRE(submit_word_tileset(&game, "beg") == 4); // valid real word (using a blank)
    REQUIRE(submit_word_tileset(&game, "abd") == 0); // valid letters, not a word
    REQUIRE(submit_word_tileset(&game, "gaze") == 2); // real word, valid letters (using two blanks)
    REQUIRE(submit_word_tileset(&game, "quad") == 3); // real word, valid letters (using two blanks)
  }

  SUBTEST("find blanks") {
    struct Game game;
    reset_tileset(&game, 0);
    memcpy(game.letters, "quit  q", 7);

    // no blanks needed
    {
      char blanks[SIZE];
      find_blanks_tileset(&game, "quit", blanks);
      for (int i = 0; i < SIZE; i++) {
        REQUIRE(blanks[i] == 0);
      }
    }

    // one blank required
    {
      char blanks[SIZE];
      find_blanks_tileset(&game, "quite", blanks);
      REQUIRE(blanks[4] == 1);
      for (int i = 0; i < SIZE; i++) {
        if (i != 4) {
          REQUIRE(blanks[i] == 0);
        }
      }
    }

    // two blanks required
    {
      char blanks[SIZE];
      find_blanks_tileset(&game, "quiter", blanks);
      REQUIRE(blanks[4] == 1);
      REQUIRE(blanks[5] == 1);
      for (int i = 0; i < SIZE; i++) {
        if (i != 4 && i != 5) {
          REQUIRE(blanks[i] == 0);
        }
      }
    }
  }

  /* check that the top scoring words are found correctly */
  SUBTEST("top words") {
    struct Game game;
    reset_tileset(&game, 0);
    memcpy(game.letters, "quitqqq", 7);
    top_words_tileset(&game);

    char best_words[STORE][SIZE] = {"quit", "qi", "tui", "it", "ti", "ut"};
    for (int i = 0; i < STORE; i++) {
      REQUIRE(strcmp(game.top_words[i], best_words[i]) == 0);
      REQUIRE(game.top_scores[i] == score_word_tileset(&game, best_words[i]));
    }
  }

  /* check that the top scoring words are found correctly, even with a blank */
  SUBTEST("top words (blanks)") {
    struct Game game;
    reset_tileset(&game, 0);
    memcpy(game.letters, "qqqqq t", 7);
    top_words_tileset(&game);

    char best_words[STORE][SIZE] = {"qat", "qi", "at", "et", "it", "st", "ta", "te", "ti", "to"};
    for (int i = 0; i < STORE; i++) {
      REQUIRE(strcmp(game.top_words[i], best_words[i]) == 0);
      REQUIRE(game.top_scores[i] == score_word_tileset(&game, best_words[i]));
    }
  }

  /* check that the top scoring words are found correctly, even with two blank */
  SUBTEST("top words (blanks)") {
    struct Game game;
    reset_tileset(&game, 0);
    memcpy(game.letters, "qqqq  q", 7);
    top_words_tileset(&game);

    char best_words[STORE][SIZE] = {"qat", "qi", "qis", "qua", "suq"};
    for (int i = 0; i < STORE; i++) {
      REQUIRE(strcmp(game.top_words[i], best_words[i]) == 0);
      REQUIRE(game.top_scores[i] == score_word_tileset(&game, best_words[i]));
    }
  }

  END_TEST();
}
