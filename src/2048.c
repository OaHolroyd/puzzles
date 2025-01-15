#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <notcurses/notcurses.h>

#include "core/logging.h"
#include "core/2048_core.h"


#define CELL_WIDTH (6)
#define CELL_HEIGHT (3)


/**
 * User interface wrapper struct.
 */
struct UI {
  struct notcurses *nc;
  struct ncplane *pln_std;
  struct ncplane *pln_board; // container for the grid
  struct ncplane *grid[SIZE * SIZE]; // grid of cells
  int alignment; // 0 = horizontal, 1 = vertical
  struct ncplane *pln_info; // info plane
};


/**
 * Draw a line in the outermost cells of a plane.
 *
 * @param ncp The plane to draw on.
 * @param gclusters Six grapheme clusters to use, ul, ur, ll, lr, hl, vl
 *        eg NCBOXLIGHT / "┌┐└┘─│".
 *
 */
void ncplane_perimiter(struct ncplane *ncp, const char *gclusters) {
  /* define color/style etc */
  unsigned ctlword = 0;
  uint32_t channels = 0;
  ncchannel_set_rgb8(&channels, 0x00, 0x00, 0x00);
  uint16_t attr = NCSTYLE_NONE;

  /* define cells */
  nccell ul = NCCELL_TRIVIAL_INITIALIZER, ur = NCCELL_TRIVIAL_INITIALIZER;
  nccell ll = NCCELL_TRIVIAL_INITIALIZER, lr = NCCELL_TRIVIAL_INITIALIZER;
  nccell hl = NCCELL_TRIVIAL_INITIALIZER, vl = NCCELL_TRIVIAL_INITIALIZER;

  int err = nccells_load_box(ncp, attr, channels, &ul, &ur, &ll, &lr, &hl, &vl, gclusters);
  if (err) {
    // fall back to ASCII
    nccells_ascii_box(ncp, attr, channels, &ul, &ur, &ll, &lr, &hl, &vl);
  }

  /* draw perimeter */
  ncplane_perimeter(ncp, &ul, &ur, &ll, &lr, &hl, &vl, ctlword);

  /* drop cells */
  nccell_release(ncp, &ul);
  nccell_release(ncp, &ur);
  nccell_release(ncp, &ll);
  nccell_release(ncp, &lr);
  nccell_release(ncp, &hl);
  nccell_release(ncp, &vl);
}


/**
 * Create a grid of cells inside a plane.
 *
 * @param ncp The parent plane containing the grid.
 * @param grid The grid of cells to draw.
 * @param rows The number of rows in the grid.
 * @param cols The number of columns in the grid.
 * @param width The width of each grid cell.
 * @param height The height of each grid cell.
 * @return 0 on success, non-zero on failure.
 */
int nc_grid(struct ncplane *ncp, struct ncplane *grid[SIZE * SIZE], int rows, int cols, int width, int height) {
  /* check that the parent plane is large enough */
  unsigned int p_rows, p_cols;
  ncplane_dim_yx(ncp, &p_rows, &p_cols);
  if (p_rows < rows * height || p_cols < cols * width) {
    LOG("ERROR: parent plane is too small for grid\n");
    return 1;
  }

  /* set all pointers to NULL to start with */
  memset(grid, 0, sizeof(struct ncplane **) * rows * cols);

  /* create the grid */
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      struct ncplane_options opts = {
        .y = i * height,
        .x = j * width,
        .rows = height,
        .cols = width,
      };
      grid[i * SIZE + j] = ncplane_create(ncp, &opts);

      if (!grid[i * SIZE + j]) {
        LOG("ERROR: failed to create grid cell %d %d\n", i, j);

        /* clean up any cells that have been created */
        for (int ii = 0; ii < rows; ii++) {
          for (int jj = 0; jj < cols; jj++) {
            if (grid[ii * SIZE + jj]) {
              ncplane_destroy(grid[ii * SIZE + jj]);
              grid[ii * SIZE + jj] = NULL;
            }
          }
        }
        return 1;
      }
    }
  }

  return 0;
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

  /* create the game board */
  struct ncplane_options opts = {
    .y = 1,
    .x = 1,
    .rows = CELL_HEIGHT * SIZE,
    .cols = CELL_WIDTH * SIZE,
  };
  ui->pln_board = ncplane_create(ui->pln_std, &opts);
  if (!ui->pln_board) {
    LOG("ERROR: failed to create board plane\n");
    return 1;
  }

  /* decide on horizontal or vertical alignment for the info pane (prefer horizontal) */
  if (cols >= 1 + CELL_WIDTH * SIZE + 9) {
    ui->alignment = 0;
    opts.y = 1;
    opts.x = 1 + CELL_WIDTH * SIZE + 1;
    opts.rows = 7;
    opts.cols = 9; // TODO: if SIZE is not 4 then this might be larger (or smaller)
  } else if (rows >= 1 + CELL_WIDTH * SIZE + 4) {
    ui->alignment = 1;
    opts.y = 1 + CELL_HEIGHT * SIZE + 1;
    opts.x = 1;
    opts.rows = 4;
    opts.cols = 16; // TODO: if SIZE is not 4 then this might be larger (or smaller)
  } else {
    // terminal isn't large enough
    ncplane_destroy(ui->pln_board);
    ui->pln_board = NULL;
    return 2;
  }

  /* set up the pane */
  ui->pln_info = ncplane_create(ui->pln_std, &opts);
  if (!ui->pln_info) {
    LOG("ERROR: failed to create info plane\n");
    ncplane_destroy(ui->pln_board);
    ui->pln_board = NULL;
    return 3;
  }

  /* add a grid over the top */
  int err = nc_grid(ui->pln_board, ui->grid, SIZE, SIZE, CELL_WIDTH, CELL_HEIGHT);
  if (err) {
    ncplane_destroy(ui->pln_board);
    ncplane_destroy(ui->pln_info);
    ui->pln_board = NULL;
    return 4;
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

  ncplane_destroy(ui->pln_board);
  ui->pln_board = NULL;

  ncplane_destroy(ui->pln_info);
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

      /* set the color */
      if (value == 0) {
        ncplane_set_bchannel(cell, 0x400000FF);
        ncplane_set_fchannel(cell, 0x4000FF00);
      } else {
        ncplane_set_bchannel(cell, 0x400FF000);
        ncplane_set_fchannel(cell, 0x40FF0000);
      }

      /* fill with spaces (to set background color) */
      for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
          ncplane_putchar_yx(cell, y, x, ' ');
        }
      }

      /* set the text for non-zero cells */
      if (value != 0) {
        char text[CELL_WIDTH];
        snprintf(text, CELL_WIDTH, "%d", 1 << value);
        ncplane_putstr_aligned(cell, 1, NCALIGN_CENTER, text);
      }
    }
  }

  /* update the info */
  ncplane_erase(ui->pln_info);
  ncplane_perimiter(ui->pln_info, NCBOXROUND);

  if (ui->alignment == 0) {
    ncplane_putstr_yx(ui->pln_info, 1, 1, "score");
    ncplane_printf_yx(ui->pln_info, 2, 1, "%d", game->score);

    ncplane_putstr_yx(ui->pln_info, 4, 1, "turn");
    ncplane_printf_yx(ui->pln_info, 5, 1, "%d", game->turn);
  } else {
    ncplane_putstr_yx(ui->pln_info, 1, 1, "score");
    ncplane_printf_yx(ui->pln_info, 2, 1, "%d", game->score);

    // TODO: if SIZE is not 4 then the x-offset might be larger (or smaller)
    ncplane_putstr_yx(ui->pln_info, 1, 9, "turn");
    ncplane_printf_yx(ui->pln_info, 2, 9, "%d", game->turn);
  }

  /* if no more moves are possible, also display a GAME OVER message */
  if (game->status != PLAYING) {
    if (ui->alignment == 0) {
      ncplane_putstr_yx(ui->pln_std, 9, SIZE * CELL_WIDTH + 2, "RETURN");
      ncplane_putstr_yx(ui->pln_std, 10, SIZE * CELL_WIDTH + 2, "TO RESET");
    } else {
      ncplane_putstr_yx(ui->pln_std, SIZE * CELL_HEIGHT + 3, 18, "RETURN");
      ncplane_putstr_yx(ui->pln_std, SIZE * CELL_HEIGHT + 4, 18, "TO RESET");
    }
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
  notcurses_options opts = {
    .termtype = NULL,
    .loglevel = NCLOGLEVEL_SILENT,
    .margin_l = 0, .margin_r = 0,
    .margin_t = 0, .margin_b = 0,
    .flags = 0 // NCOPTION_SUPPRESS_BANNERS // to suppress info at end
  };
  struct notcurses *nc = notcurses_init(&opts, stdout);

  /* set up the screen */
  int err = notcurses_enter_alternate_screen(nc);
  if (err) {
    LOG("ERROR: failed to enter alternate screen\n");
    return 1;
  }

  /* create the game board */
  struct UI ui;
  ui_setup(nc, &ui);
  if (!ui.pln_board) {
    LOG("ERROR: failed to set up UI\n");
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
    LOG("KEY PRESSED: %d [%c]", id, id);

    /* special key handling */
    if (id == 'q') {
      break;
    }

    /* handle keypress */
    Result result = MOVE_ERROR;
    switch (id) {
      case NCKEY_DOWN:
        // NOTE: up and down are mirrored
        result = game_move(&game, UP);
        break;
      case NCKEY_UP:
        // NOTE: up and down are mirrored
        result = game_move(&game, DOWN);
        break;
      case NCKEY_LEFT:
        result = game_move(&game, LEFT);
        break;
      case NCKEY_RIGHT:
        result = game_move(&game, RIGHT);
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
  notcurses_leave_alternate_screen(nc);
  notcurses_stop(nc);

  return 0;
}
