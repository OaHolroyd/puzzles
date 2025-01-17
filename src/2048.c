#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <notcurses/notcurses.h>

#include "core/logging.h"
#include "core/nc-utils.h"
#include "core/game_2048.h"


#define CELL_WIDTH (7)
#define CELL_HEIGHT (3)


/**
 * User interface wrapper struct.
 */
struct UI {
  struct notcurses *nc;
  struct ncplane *pln_std;
  struct ncplane *pln_border;
  struct ncplane *pln_board; // container for the grid
  struct ncplane *grid[SIZE * SIZE]; // grid of cells
  int alignment; // 0 = horizontal, 1 = vertical
  struct ncplane *pln_info; // info plane
};


/**
 * Set the background/foreground colors of a cell based on its value.
 *
 * @param cell The cell to set the colors of.
 * @param value The value of the cell (the exponent of 2).
 */
void set_cell_colors(struct ncplane *cell, int value) {
  static_assert(SIZE == 4, "if SIZE != 4 then the colors are not set correctly");

  /* define a color array so we can use value as an index */
  // NOTE: the first color is the background color, the second is the text color
  uint32_t hex_colors[19][2] = {
    // shades of grey up to 1024
    {0x000000, 0x000000}, // 0 (blank)
    {0x202020, 0xFFFFFF}, // 1 (2)
    {0x404040, 0xFFFFFF}, // 2 (4)
    {0x606060, 0xFFFFFF}, // 3 (8)
    {0x808080, 0xFFFFFF}, // 4 (16)
    {0xA0A0A0, 0xFFFFFF}, // 5 (32)
    {0xB0B0B0, 0xFFFFFF}, // 6 (64)
    {0xC0C0C0, 0x000000}, // 7 (128)
    {0xD0D0D0, 0x000000}, // 8 (256)
    {0xE0E0E0, 0x000000}, // 9 (512)
    {0xF0F0F0, 0x000000}, // 10 (1024)

    // colors for 2048 and above
    {0xFFA600, 0xFFFFFF}, // 11 (2048)
    {0xFF4500, 0xFFFFFF}, // 12 (4096)
    {0x8B0000, 0xFFFFFF}, // 13 (8192)
    {0x550A35, 0xFFFFFF}, // 14 (16384)
    {0x6A0DAD, 0xFFFFFF}, // 15 (32768)
    {0x5539EC, 0xFFFFFF}, // 16 (65536)
    {0x0909FF, 0xFFFFFF}, // 17 (131072)

    // special color that should never be reached
    {0x00FFFF, 0xFF0000}, // 18 error
  };

  if (value < 0 || value > 17) {
    value = 0;
  }

  ncplane_set_bchannel(cell, 0x40000000 + hex_colors[value][0]);
  ncplane_set_fchannel(cell, 0x40000000 + hex_colors[value][1]);
}


/**
 * Set up the user interface.
 *
 * @param nc The notcurses context.
 * @param ui The user interface to set up.
 * @return 0 on success, non-zero on failure.
 */
int ui_setup(struct notcurses *nc, struct UI *ui) {
  ui->nc = nc;
  ui->pln_std = notcurses_stdplane(nc);
  ui->pln_board = NULL;

  /* get the dimensions of the standard plane */
  unsigned int rows, cols;
  ncplane_dim_yx(ui->pln_std, &rows, &cols);

  /* black background, white foreground */
  ncplane_set_bchannel(ui->pln_std, 0x40000000);
  ncplane_set_fchannel(ui->pln_std, 0x40FFFFFF);
  ncutil_fill(ui->pln_std, ' ');

  /* border around the board */
  struct ncplane_options opts;
  opts.y = 0;
  opts.x = 0;
  opts.rows = CELL_HEIGHT * SIZE + 2;
  opts.cols = CELL_WIDTH * SIZE + 2;
  ui->pln_border = ncplane_create(ui->pln_std, &opts);
  if (!ui->pln_border) {
    LOG("ERROR: failed to create border plane");
    return 1;
  }
  LOG("INFO: created border plane");
  ncutil_perimiter(ui->pln_border, NCBOXROUND);

  /* create the game board */
  opts.y = 1;
  opts.x = 1;
  opts.rows = CELL_HEIGHT * SIZE;
  opts.cols = CELL_WIDTH * SIZE;
  ui->pln_board = ncplane_create(ui->pln_std, &opts);
  if (!ui->pln_board) {
    LOG("ERROR: failed to create board plane");
    ncplane_destroy(ui->pln_border);
    ui->pln_border = NULL;
    return 1;
  }
  LOG("INFO: created board plane");

  /* decide on horizontal or vertical alignment for the info pane (prefer horizontal) */
  static_assert(SIZE == 4, "if SIZE != 4 then the max score width is different");
  if (cols >= 1 + CELL_WIDTH * SIZE + 9) {
    ui->alignment = 0;
    opts.y = 0;
    opts.x = 1 + CELL_WIDTH * SIZE + 1;
    opts.rows = 7;
    opts.cols = 9; // this is the max score width + 2 (7 + 2)
  } else if (rows >= 1 + CELL_HEIGHT * SIZE + 4) {
    ui->alignment = 1;
    opts.y = 1 + CELL_HEIGHT * SIZE + 1;
    opts.x = 0;
    opts.rows = 4;
    opts.cols = 16; // this is max score width + max turn width + 3 (7 + 6 + 3)
  } else {
    // terminal isn't large enough
    LOG("ERROR: terminal too small for info plane");
    ncplane_destroy(ui->pln_border);
    ui->pln_border = NULL;
    ncplane_destroy(ui->pln_board);
    ui->pln_board = NULL;
    return 2;
  }

  /* set up the info pane */
  ui->pln_info = ncplane_create(ui->pln_std, &opts);
  if (!ui->pln_info) {
    LOG("ERROR: failed to create info plane");
    ncplane_destroy(ui->pln_border);
    ui->pln_border = NULL;
    ncplane_destroy(ui->pln_board);
    ui->pln_board = NULL;
    return 3;
  }
  LOG("INFO: created info plane");
  // ncplane_set_bchannel(ui->pln_info, 0x40000000);
  // ncplane_set_fchannel(ui->pln_info, 0x40FFFFFF);
  // ncutil_fill(ui->pln_info, ' ');

  /* add a grid over the top */
  int err = ncutil_grid(ui->pln_board, ui->grid, SIZE, SIZE, CELL_WIDTH, CELL_HEIGHT);
  if (err) {
    // failed to create the grid
    ncplane_destroy(ui->pln_border);
    ui->pln_border = NULL;
    ncplane_destroy(ui->pln_board);
    ui->pln_board = NULL;
    ncplane_destroy(ui->pln_info);
    ui->pln_info = NULL;
    return 4;
  }
  LOG("INFO: created grid plane");

  /* show instructions */
  if (ui->alignment == 0) {
    ncplane_putstr_yx(ui->pln_std, 8, SIZE * CELL_WIDTH + 2, "[R]ESET");
    ncplane_putstr_yx(ui->pln_std, 9, SIZE * CELL_WIDTH + 2, "[Q]UIT");
  } else {
    ncplane_putstr_yx(ui->pln_std, SIZE * CELL_HEIGHT + 3, 17, "[R]ESET");
    ncplane_putstr_yx(ui->pln_std, SIZE * CELL_HEIGHT + 4, 17, "[Q]UIT");
  }

  return 0;
}


/**
 * Free the memory used by the user interface.
 *
 * @param ui The user interface.
 */
void ui_destroy(struct UI *ui) {
  for (int k = 0; k < SIZE * SIZE; k++) {
    if (ui->grid[k]) {
      ncplane_destroy(ui->grid[k]);
      ui->grid[k] = NULL;
    }
  }

  if (ui->pln_border) {
    ncplane_destroy(ui->pln_border);
  }
  ui->pln_border = NULL;

  if (ui->pln_board) {
    ncplane_destroy(ui->pln_board);
  }
  ui->pln_board = NULL;

  if (ui->pln_info) {
    ncplane_destroy(ui->pln_info);
  }
  ui->pln_info = NULL;
}


/**
 * Render the game board.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
void ui_render(struct UI *ui, struct Game *game) {
  /* set the color and text of the grid cells */
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      struct ncplane *cell = ui->grid[i * SIZE + j];
      int value = game->grid[i * SIZE + j];
      unsigned height, width;
      ncplane_dim_yx(cell, &height, &width);

      /* fill with spaces (to set background color) */
      set_cell_colors(cell, value);
      ncutil_fill(cell, ' ');

      /* set the text for non-zero cells */
      if (value != 0) {
        char text[CELL_WIDTH];
        snprintf(text, CELL_WIDTH, "%d", 1 << value);
        ncplane_putstr_aligned(cell, 1, NCALIGN_CENTER, text);
      }
    }
  }

  /* update the info */
  ncutil_perimiter(ui->pln_info, NCBOXROUND);

  if (ui->alignment == 0) {
    ncplane_putstr_yx(ui->pln_info, 1, 1, "score");
    ncplane_printf_yx(ui->pln_info, 2, 1, "%d", game->score);

    ncplane_putstr_yx(ui->pln_info, 4, 1, "turn");
    ncplane_printf_yx(ui->pln_info, 5, 1, "%d", game->turn);
  } else {
    ncplane_putstr_yx(ui->pln_info, 1, 1, "score");
    ncplane_printf_yx(ui->pln_info, 2, 1, "%d", game->score);

    ncplane_putstr_yx(ui->pln_info, 1, 9, "turn");
    ncplane_printf_yx(ui->pln_info, 2, 9, "%d", game->turn);
  }

  notcurses_render(ui->nc);
}


int main(int argc, char const *argv[]) {
  /* set up logging */
  log_start("2048.log");

  /* set up a 2048 game state */
  srand(time(NULL));
  struct Game game;
  game_reset(&game);

  /* start notcurses */
  struct notcurses *nc = ncu_start();
  LOG("INFO: started notcurses");
  if (nc == NULL) {
    return 1;
  }

  /* create the game board */
  struct UI ui;
  ui_setup(nc, &ui);
  if (!ui.pln_board) {
    LOG("ERROR: failed to set up UI");
    notcurses_leave_alternate_screen(nc);
    notcurses_stop(nc);
    return 1;
  }

  /* render the screen before starting the game */
  ui_render(&ui, &game);

  /* game loop */
  ncinput input = {0};
  uint32_t id_prev = 0;
  while (1) {
    uint32_t id = notcurses_get_blocking(nc, &input);
    LOG("INFO: key pressed: %d [%c]", id, id);

    /* special key handling */
    if (id == 'q' || id == 'Q') {
      break;
    }

    /* handle keypress */
    Result result = MOVE_ERROR;
    switch (id) {
      case NCKEY_DOWN:
        result = game_turn(&game, DOWN);
        break;
      case NCKEY_UP:
        result = game_turn(&game, UP);
        break;
      case NCKEY_LEFT:
        result = game_turn(&game, LEFT);
        break;
      case NCKEY_RIGHT:
        result = game_turn(&game, RIGHT);
        break;
      case NCKEY_ENTER:
        // reset the game if enter is pressed twice or if game is over
        if (id_prev == NCKEY_ENTER || game.status != PLAYING) {
          game_reset(&game);
          result = MOVE_SUCCESS;
        }
        break;
      default:
        // do nothing
        break;
    }

    /* render the screen if there were any updates */
    if (result != MOVE_ERROR) {
      ui_render(&ui, &game);
    }

    id_prev = id;
  }

  /* clean up resources */
  ui_destroy(&ui);
  ncu_end(nc);

  return 0;
}
