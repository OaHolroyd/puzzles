#include "testing.h"

#include <string.h>

#include <core/game_2048.h>


int main(void) {
  START_TEST("test_2048");
  REQUIRE(SIZE * SIZE == 16);

  struct Game game;

  /* basic single pair */
  {
    int grid0[16] = {
      0, 0, 1, 0,
      0, 0, 1, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
    };

    memcpy(game.grid, grid0, 16 * sizeof(int));
    REQUIRE(move_2048(&game, UP) == MOVE_SUCCESS);
    int answer00[16] = {
      0, 0, 2, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer00[i] == game.grid[i]);

    memcpy(game.grid, grid0, 16 * sizeof(int));
    REQUIRE(move_2048(&game, DOWN) == MOVE_SUCCESS);
    int answer01[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 2, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer01[i] == game.grid[i]);

    memcpy(game.grid, grid0, 16 * sizeof(int));
    REQUIRE(move_2048(&game, LEFT) == MOVE_SUCCESS);
    int answer02[16] = {
      1, 0, 0, 0,
      1, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer02[i] == game.grid[i]);

    memcpy(game.grid, grid0, 16 * sizeof(int));
    REQUIRE(move_2048(&game, RIGHT) == MOVE_SUCCESS);
    int answer03[16] = {
      0, 0, 0, 1,
      0, 0, 0, 1,
      0, 0, 0, 0,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer03[i] == game.grid[i]);
  }

  /* horizontal triple (so order of merge matters) */
  {
    int grid1[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 1, 1, 1,
    };

    memcpy(game.grid, grid1, 16 * sizeof(int));
    REQUIRE(move_2048(&game, UP) == MOVE_SUCCESS);
    int answer10[16] = {
      0, 1, 1, 1,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer10[i] == game.grid[i]);

    memcpy(game.grid, grid1, 16 * sizeof(int));
    REQUIRE(move_2048(&game, DOWN) == MOVE_ERROR);
    int answer11[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 1, 1, 1,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer11[i] == game.grid[i]);

    memcpy(game.grid, grid1, 16 * sizeof(int));
    REQUIRE(move_2048(&game, LEFT) == MOVE_SUCCESS);
    int answer12[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      2, 1, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer12[i] == game.grid[i]);

    memcpy(game.grid, grid1, 16 * sizeof(int));
    REQUIRE(move_2048(&game, RIGHT) == MOVE_SUCCESS);
    int answer13[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 1, 2,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer13[i] == game.grid[i]);


    /* horizontal triple with gap (should match above) */
    int grid2[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      1, 1, 0, 1,
    };

    memcpy(game.grid, grid2, 16 * sizeof(int));
    REQUIRE(move_2048(&game, LEFT) == MOVE_SUCCESS);
    for (int i = 0; i < 16; i++) REQUIRE(answer12[i] == game.grid[i]);

    memcpy(game.grid, grid2, 16 * sizeof(int));
    REQUIRE(move_2048(&game, RIGHT) == MOVE_SUCCESS);
    for (int i = 0; i < 16; i++) REQUIRE(answer13[i] == game.grid[i]);
  }

  /* vertical triple (so order of merge matters) */
  {
    int grid3[16] = {
      0, 1, 0, 0,
      0, 1, 0, 0,
      0, 1, 0, 0,
      0, 0, 0, 0,
    };

    memcpy(game.grid, grid3, 16 * sizeof(int));
    REQUIRE(move_2048(&game, UP) == MOVE_SUCCESS);
    int answer30[16] = {
      0, 2, 0, 0,
      0, 1, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer30[i] == game.grid[i]);

    memcpy(game.grid, grid3, 16 * sizeof(int));
    REQUIRE(move_2048(&game, DOWN) == MOVE_SUCCESS);
    int answer31[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 1, 0, 0,
      0, 2, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer31[i] == game.grid[i]);

    memcpy(game.grid, grid3, 16 * sizeof(int));
    REQUIRE(move_2048(&game, LEFT) == MOVE_SUCCESS);
    int answer32[16] = {
      1, 0, 0, 0,
      1, 0, 0, 0,
      1, 0, 0, 0,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer32[i] == game.grid[i]);

    memcpy(game.grid, grid3, 16 * sizeof(int));
    REQUIRE(move_2048(&game, RIGHT) == MOVE_SUCCESS);
    int answer33[16] = {
      0, 0, 0, 1,
      0, 0, 0, 1,
      0, 0, 0, 1,
      0, 0, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer33[i] == game.grid[i]);


    /* vertical triple with gap (should match above) */
    int grid4[16] = {
      0, 1, 0, 0,
      0, 1, 0, 0,
      0, 0, 0, 0,
      0, 1, 0, 0,
    };

    memcpy(game.grid, grid4, 16 * sizeof(int));
    REQUIRE(move_2048(&game, UP) == MOVE_SUCCESS);
    for (int i = 0; i < 16; i++) REQUIRE(answer30[i] == game.grid[i]);

    memcpy(game.grid, grid4, 16 * sizeof(int));
    REQUIRE(move_2048(&game, DOWN) == MOVE_SUCCESS);
    for (int i = 0; i < 16; i++) REQUIRE(answer31[i] == game.grid[i]);
  }

  /* full grid (valid plays) */
  {
    /* full grid (with valid plays) */
    int grid5[16] = {
      2, 1, 1, 1,
      2, 2, 2, 1,
      4, 1, 2, 2,
      5, 2, 1, 3,
    };

    memcpy(game.grid, grid5, 16 * sizeof(int));
    REQUIRE(move_2048(&game, UP) == MOVE_SUCCESS);
    int answer50[16] = {
      3, 1, 1, 2,
      4, 2, 3, 2,
      5, 1, 1, 3,
      0, 2, 0, 0,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer50[i] == game.grid[i]);

    memcpy(game.grid, grid5, 16 * sizeof(int));
    REQUIRE(move_2048(&game, DOWN) == MOVE_SUCCESS);
    int answer51[16] = {
      0, 1, 0, 0,
      3, 2, 1, 2,
      4, 1, 3, 2,
      5, 2, 1, 3,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer51[i] == game.grid[i]);

    memcpy(game.grid, grid5, 16 * sizeof(int));
    REQUIRE(move_2048(&game, LEFT) == MOVE_SUCCESS);
    int answer52[16] = {
      2, 2, 1, 0,
      3, 2, 1, 0,
      4, 1, 3, 0,
      5, 2, 1, 3,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer52[i] == game.grid[i]);

    memcpy(game.grid, grid5, 16 * sizeof(int));
    REQUIRE(move_2048(&game, RIGHT) == MOVE_SUCCESS);
    int answer53[16] = {
      0, 2, 1, 2,
      0, 2, 3, 1,
      0, 4, 1, 3,
      5, 2, 1, 3,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer53[i] == game.grid[i]);
  }

  /* full grid (no valid plays) */
  {
    int grid6[16] = {
      2, 1, 2, 3,
      1, 2, 3, 4,
      2, 1, 2, 1,
      1, 2, 3, 4,
    };

    memcpy(game.grid, grid6, 16 * sizeof(int));
    REQUIRE(move_2048(&game, UP) == MOVE_ERROR);
    int answer60[16] = {
      2, 1, 2, 3,
      1, 2, 3, 4,
      2, 1, 2, 1,
      1, 2, 3, 4,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer60[i] == game.grid[i]);

    memcpy(game.grid, grid6, 16 * sizeof(int));
    REQUIRE(move_2048(&game, DOWN) == MOVE_ERROR);
    int answer61[16] = {
      2, 1, 2, 3,
      1, 2, 3, 4,
      2, 1, 2, 1,
      1, 2, 3, 4,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer61[i] == game.grid[i]);

    memcpy(game.grid, grid6, 16 * sizeof(int));
    REQUIRE(move_2048(&game, LEFT) == MOVE_ERROR);
    int answer62[16] = {
      2, 1, 2, 3,
      1, 2, 3, 4,
      2, 1, 2, 1,
      1, 2, 3, 4,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer62[i] == game.grid[i]);

    memcpy(game.grid, grid6, 16 * sizeof(int));
    REQUIRE(move_2048(&game, RIGHT) == MOVE_ERROR);
    int answer63[16] = {
      2, 1, 2, 3,
      1, 2, 3, 4,
      2, 1, 2, 1,
      1, 2, 3, 4,
    };
    for (int i = 0; i < 16; i++) REQUIRE(answer63[i] == game.grid[i]);
  }

  END_TEST();
}
