//
// Created by Oscar Holroyd on 15/01/2025.
//

#ifndef GAME_2048_H
#define GAME_2048_H

#define SIZE (4) // size of the grid


/**
 * Game status.
 */
typedef enum GameStatus {
  PLAYING,
  LOST,
} GameStatus;


/**
 * Possible moves.
 */
typedef enum Move {
  UP,
  DOWN,
  LEFT,
  RIGHT,
} Move;


/**
 * Possible results of a move.
 */
typedef enum Result {
  MOVE_SUCCESS,
  MOVE_ERROR,
  MOVE_GAMEOVER,
} Result;


/**
 * Game state.
 */
struct Game {
  int grid[SIZE * SIZE]; // grid with numbers represented by powers of 2
  int score; // current score
  int turn; // current turn
  GameStatus status; // game status

  int nz; // number of zero cells
  int merge[SIZE * SIZE]; // merging info
};


/**
 * Reset the game state.
 *
 * @param game The game state.
 */
void game_reset(struct Game *game);


/**
 * Move the tiles in the given direction.
 *
 * @param game The game state.
 * @param move The direction to move the tiles.
 * @return The result of the move.
 */
Result game_move(struct Game *game, Move move);


#endif //GAME_2048_H
