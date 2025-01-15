//
// Created by Oscar Holroyd on 15/01/2025.
//

#include "nc-utils.h"

#include "logging.h"


ncblitter_e get_blitter(struct notcurses *nc) {
  if (notcurses_canpixel(nc)) {
    // pixels are the best option
    return NCBLIT_PIXEL;
  }

  return NCBLIT_DEFAULT;
}


ncscale_e get_scale(ncblitter_e blitter) {
  if (blitter == NCBLIT_PIXEL) {
    return NCSCALE_SCALE_HIRES;
  }

  return NCSCALE_NONE;
}


void ncutil_perimiter(struct ncplane *ncp, const char *gclusters) {
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


void ncutil_fill(struct ncplane *ncp, char ch) {
  unsigned int rows, cols;
  ncplane_dim_yx(ncp, &rows, &cols);

  for (unsigned y = 0; y < rows; y++) {
    for (unsigned x = 0; x < cols; x++) {
      ncplane_putchar_yx(ncp, y, x, ch);
    }
  }
}


int ncutil_grid(struct ncplane *ncp, struct ncplane **grid, int rows, int cols, int width, int height) {
  /* check that the parent plane is large enough */
  unsigned int p_rows, p_cols;
  ncplane_dim_yx(ncp, &p_rows, &p_cols);
  if (p_rows < rows * height || p_cols < cols * width) {
    LOG("ERROR: parent plane is too small for grid\n");
    return 1;
  }

  /* set all pointers to NULL to start with */
  memset(grid, 0, sizeof(struct ncplane *) * rows * cols);

  /* create the grid */
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      struct ncplane_options opts = {
        .y = i * height,
        .x = j * width,
        .rows = height,
        .cols = width,
      };
      grid[i * cols + j] = ncplane_create(ncp, &opts);

      if (!grid[i * cols + j]) {
        LOG("ERROR: failed to create grid cell %d %d\n", i, j);

        /* clean up any cells that have been created */
        for (int ii = 0; ii < rows; ii++) {
          for (int jj = 0; jj < cols; jj++) {
            if (grid[ii * cols + jj]) {
              ncplane_destroy(grid[ii * cols + jj]);
              grid[ii * cols + jj] = NULL;
            }
          }
        }
        return 1;
      }
    }
  }

  return 0;
}
