//
// Created by Oscar Holroyd on 15/01/2025.
//

#include "game_2048.h"

#include <stdlib.h>
#include <string.h>


/**
 * Perform a single move/merge step
 *
 * @param game The game state.
 * @param move The move to perform.
 * @return 0 if no move was made, 1 otherwise.
 */
int movemerge(struct Game *game, const Move move) {
  int has_moved = 0;

  /* set up the loops depending on the move direction */
  int i0 = 0, i1 = 0, j0 = 0, j1 = 0, di = 0, dj = 0, d = 0;
  switch (move) {
    case UP:
      i0 = 0;
      i1 = SIZE - 1;
      di = 1; // bottom to top
      j0 = 0;
      j1 = SIZE;
      dj = 1; // left to right
      d = SIZE; // next row
      break;
    case LEFT:
      i0 = 0;
      i1 = SIZE;
      di = 1; // bottom to top
      j0 = 0;
      j1 = SIZE - 1;
      dj = 1; // left to right
      d = 1; // next column
      break;
    case DOWN:
      i0 = SIZE - 1;
      i1 = 0;
      di = -1; // top to bottom
      j0 = SIZE - 1;
      j1 = -1;
      dj = -1; // right to left
      d = -SIZE; // previous row
      break;
    case RIGHT:
      i0 = SIZE - 1;
      i1 = -1;
      di = -1; // top to bottom
      j0 = SIZE - 1;
      j1 = 0;
      dj = -1; // right to left
      d = -1; // previous column
      break;
  }

  /* move/merge cycle */
  for (int i = i0; i != i1; i += di) {
    for (int j = j0; j != j1; j += dj) {
      int *cell = game->grid + j + SIZE * i;
      int *mcell = game->merge + j + SIZE * i;

      /* empty space: move into it */
      if (*cell == 0) {
        if (*(cell + d) != 0) {
          *cell = *(cell + d);
          *mcell = *(mcell + d);

          *(cell + d) = 0;
          *(mcell + d) = 0;

          has_moved = 1;
        }
      }

      /* match next: merge into it */
      else if ((*cell == *(cell + d)) && (*mcell == 0) && (*(mcell + d) == 0)) {
        *cell += 1;
        *mcell = 1;
        game->score += (1 << *cell) * (*cell - 1);
        game->nz++;

        *(cell + d) = 0;
        *(mcell + d) = 0;

        has_moved = 1;
      }
    } // j end
  } // i end

  return has_moved;
}


/**
 * Fills a random cell in the grid with a (biased) random value.
 *
 * @param game The game state.
 * @return The index of the added tile.
 */
int fill_random_cell(struct Game *game) {
  /* choose a random empty cell */
  int n = rand() % game->nz;
  game->nz--;

  /* insert 2 or 4 into the grid */
  for (int i = 0; i < SIZE * SIZE; i++) {
    if (game->grid[i] == 0) {
      /* reached the selected cell */
      if (n == 0) {
        /* 90% change of a 2, 10% change of a 4 */
        if (rand() % 10) {
          game->grid[i] = 1;
        } else {
          game->grid[i] = 2;
        }
        return i;
      }
      n--; // otherwise count down another empty cell
    }
  } // i end

  return -1;
}


void set_status(struct Game *game) {
  /* if there are any non-zero cells then there must be a possible move */
  if (game->nz) {
    game->status = PLAYING;
  }

  /* check up/down matches */
  for (int i = 0; i < SIZE - 1; i++) {
    for (int j = 0; j < SIZE; j++) {
      const int *cell = game->grid + j + SIZE * i;
      if (*cell == *(cell + SIZE)) {
        game->status = PLAYING;
        return;
      }
    } // j end
  } // i end

  /* check left/right matches */
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE - 1; j++) {
      const int *cell = game->grid + j + SIZE * i;
      if (*cell == *(cell + 1)) {
        game->status = PLAYING;
        return;
      }
    } // j end
  } // i end

  game->status = LOST;
}


void game_reset(struct Game *game) {
  game->score = 0;
  game->turn = 0;
  game->status = PLAYING;

  /* clear the grid */
  memset(game->grid, 0, SIZE*SIZE*sizeof(game->grid[0]));
  game->nz = SIZE * SIZE;

  /* start with two filled cells */
  fill_random_cell(game);
  fill_random_cell(game);
}


Result game_move(struct Game *game, const Move move) {
  memset(game->merge, 0, SIZE*SIZE*sizeof(game->merge[0])); // reset merge info

  /* move/merge until no more moves/merges are possible */
  int total_cycles = 0;
  int has_moved;
  do {
    has_moved = movemerge(game, move);
    total_cycles += has_moved;
  } while (has_moved);

  /* no cycles performed means the move did nothing */
  if (total_cycles == 0) {
    return MOVE_ERROR;
  }

  return MOVE_SUCCESS;
}


Result game_turn(struct Game *game, const Move move) {
  const Result result = game_move(game, move);

  if (result == MOVE_ERROR) {
    return result;
  }

  fill_random_cell(game);
  game->turn++;

  /* check if the game is over */
  set_status(game);
  if (game->status != PLAYING) {
    return MOVE_GAMEOVER;
  }

  return result;
}
