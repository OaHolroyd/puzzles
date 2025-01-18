#include "testing.h"

#include <string.h>

#include <core/game_tileset.h>


int main(void) {
  START_TEST("tileset");

  /* check that a shuffle always keeps the letters the same */
  SUBTEST("shuffle") {
    struct Game game;
    reset_tileset(&game);

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
    reset_tileset(&game);

    for (int i = 0; i < 500000; i++) {
      reset_tileset(&game);

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

  END_TEST();
}
