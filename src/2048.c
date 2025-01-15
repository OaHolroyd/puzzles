#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <notcurses/notcurses.h>

#include "core/logging.h"
#include "core/2048_core.h"


/**
 * User interface wrapper struct.
 */
struct UI {
  struct notcurses *nc;
  struct ncplane *pln_std;
  struct ncplane *pln_board;
  struct ncplane *grid[SIZE * SIZE];
};


/**
 * Get the best blitter available for the terminal.
 *
 * @param nc The notcurses context.
 * @return The best blitter.
 */
ncblitter_e get_blitter(struct notcurses *nc) {
  if (notcurses_canpixel(nc)) {
    // pixels are the best option
    return NCBLIT_PIXEL;
  }

  return NCBLIT_DEFAULT;
}

/**
 * Get the correct scaling option for a given blitter.
 *
 * @param blitter The blitter to use.
 * @return The best scaling option.
 */
ncscale_e get_scale(ncblitter_e blitter) {
  if (blitter == NCBLIT_PIXEL) {
    return NCSCALE_SCALE_HIRES;
  }

  return NCSCALE_NONE;
}


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
int ncgrid(struct ncplane *ncp, struct ncplane *grid[SIZE * SIZE], int rows, int cols, int width, int height) {
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
int set_up_ui(struct notcurses *nc, struct UI *ui) {
  ui->nc = nc;
  ui->pln_std = notcurses_stdplane(nc);
  ui->pln_board = NULL;

  /* get the dimensions of the standard plane */
  unsigned int rows, cols;
  ncplane_dim_yx(ui->pln_std, &rows, &cols);

  /* cell setup */
  const int cell_width = 6;
  const int cell_height = 3;

  /* create the game board */
  struct ncplane_options opts = {
    .y = 2,
    .x = 2,
    .rows = cell_height * SIZE,
    .cols = cell_width * SIZE,
  };
  ui->pln_board = ncplane_create(ui->pln_std, &opts);
  if (!ui->pln_board) {
    LOG("ERROR: failed to create board plane\n");
    return 1;
  }

  /* add a grid over the top */
  int err = ncgrid(ui->pln_board, ui->grid, SIZE, SIZE, cell_width, cell_height);
  if (err) {
    ncplane_destroy(ui->pln_board);
    ui->pln_board = NULL;
    return 2;
  }

  return 0;
}


/**
 * Render the game board.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
void render_ui(struct UI *ui, struct Game *game) {
  /* set the color of the grid cells according to their number */
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

      /* set the text */
      char text[16];
      snprintf(text, 16, "%d", value);
      ncplane_putstr_aligned(cell, 1, NCALIGN_CENTER, text);
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

  /* check if images can be loaded */
  int canopen_images = notcurses_canopen_images(nc);
  ncblitter_e blitter = get_blitter(nc);
  ncscale_e scale = get_scale(blitter);

  /* set up the screen */
  int err = notcurses_enter_alternate_screen(nc);
  if (err) {
    LOG("ERROR: failed to enter alternate screen\n");
    return 1;
  }

  /* create the game board */
  struct UI ui;
  set_up_ui(nc, &ui);
  if (!ui.pln_board) {
    LOG("ERROR: failed to set up UI\n");
    notcurses_leave_alternate_screen(nc);
    notcurses_stop(nc);
    return 1;
  }

  /* render the screen before starting the game */
  render_ui(&ui, &game);

  /* game loop */
  ncinput input = {0};
  while (1) {
    uint32_t id = notcurses_get_blocking(nc, &input);
    LOG("KEY PRESSED: %d [%c]", id, id);

    /* special key handling */
    if (id == NCKEY_ESC) {
      break;
    }

    /* handle keypress */
    Result result = MOVE_ERROR;
    switch (id) {
      // NOTE: up and down are mirrored
      case NCKEY_DOWN:
        result = game_move(&game, UP);
        break;
      case NCKEY_UP:
        result = game_move(&game, DOWN);
      break;
      case NCKEY_LEFT:
        result = game_move(&game, LEFT);
        break;
      case NCKEY_RIGHT:
        result = game_move(&game, RIGHT);
        break;
      default:
        // do nothing
        break;
    }

    /* render the screen if there were any updates */
    if (result == MOVE_SUCCESS) {
      render_ui(&ui, &game);
    }
  }

  // TODO: destroy any created planes

  notcurses_leave_alternate_screen(nc);
  notcurses_stop(nc);

  return 0;
}
